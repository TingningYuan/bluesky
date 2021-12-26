#include "fiber.h"
#include "config.h"
#include "macro.h"

namespace bluesky
{
    static std::atomic<uint64_t> sFiberID{0};
    static std::atomic<uint64_t> sFiberCount{0};
   
    //当前线程正在运行的协程
    static thread_local Fiber* tCurFiber = nullptr;
    //主协程,切换到这个协程就相当于切换到主线程中运行
    static thread_local Fiber::Ptr tMainFiber = nullptr;

    static ConfigVar<uint32_t>::Ptr g_fiber_stack_size =
        Config::lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

    class MallocStackAllocator
    {
   public:
       static void* Alloc(size_t size)
       {
           return std::malloc(size);
       }
       static void Dealloc(void* ptr, size_t size)
       {
           std::free(ptr);
       }
    };
    using StackAllocator = MallocStackAllocator;
    //默认构造函数就是这个线程的主协程，没有任何参数
    Fiber::Fiber()
    {
        state_ = EXEC;
        set_current_fiber(this);
        if (getcontext(&ctx_))
        {
            BLUESKY_ASSERT2(false, "getcontext");
        }
        ++sFiberCount;
        BLUESKY_LOG_DEBUG(BLUESKY_LOG_NAME("system")) << "Fiber::Fiber() main id = " << id_;
    }

    //开启新的协程，分配栈空间，每个协程都拥有独立的栈
    Fiber::Fiber(std::function<void()> callback, size_t stacksize)
        :callback_(callback),id_(++sFiberID)
    {
        sFiberCount++;
        //1:分配空间
        stacksize_ = stacksize ? stacksize_ : g_fiber_stack_size->get_value();
        stack_ = StackAllocator::Alloc(stacksize_);
        state_ = INIT;
        if(getcontext(&ctx_)){
            BLUESKY_ASSERT2(false, "Fiber::getcontex()");
        }
        //2:设置u_context
        //当前上下文结束后，下一个激活的上下文对象的指针，只在当前上下文是由makecontext创建时有效
        ctx_.uc_link = nullptr;
        // 当前上下文使用的栈内存空间，只在当前上下文是由makecontext创建时有效
        ctx_.uc_stack.ss_sp = stack_;
        ctx_.uc_stack.ss_size = stacksize_;

        makecontext(&ctx_, &main_func, 0);
        BLUESKY_LOG_DEBUG(BLUESKY_LOG_NAME("system"))<<"Fiber::Fiber(callback,stacksize)";
    }

    //在协程结束后，回收它的内存
    Fiber::~Fiber()
    {
        sFiberCount--;
        /*
        BLUESKY_LOG_DEBUG(BLUESKY_LOG_NAME("system"))<<"~Fiber::"<<this->get_fiberID()
                                                    <<"this state_:"<<this->state_to_string(this->state_)
                                                    <<" and tCurFiber state_: "<<tCurFiber->state_to_string(tCurFiber->state_)\
                                                   <<" and tMainFiber state_: "<<tMainFiber->state_to_string(tMainFiber->state_);
        */
        //如果当前协程栈不为空
        if(stack_){
            BLUESKY_ASSERT2(state_ != EXEC, this->state_to_string(state_));
            StackAllocator::Dealloc(stack_, stacksize_);
            BLUESKY_LOG_DEBUG(BLUESKY_LOG_NAME("system"))<<"~Fiber::stack not empty";
        }
        else{
            //若回调函数不为空
            BLUESKY_ASSERT(!callback_);
            //协程正在执行,则将当前协程设置为nullptr
            BLUESKY_ASSERT(state_ == EXEC || state_==READY || state_==HOLD);

            //Fiber::Ptr cur(tCurFiber);
             
            if(tCurFiber==this)
            {
                set_current_fiber(nullptr);
            }
            BLUESKY_LOG_DEBUG(BLUESKY_LOG_NAME("system"))<<"~Fiber::stack empty";
        }
        BLUESKY_LOG_DEBUG(BLUESKY_LOG_NAME("system"))<<"Fiber::~Fiber() end";
    }

    //为了充分的利用内存，一个协程结束后，它的内存还没有被释放
    //因此可以利用这内存重新初始化一个新的协程
    void Fiber::reset(std::function<void()> callback)
    {
        BLUESKY_ASSERT(stack_);
        BLUESKY_ASSERT(state_==TERM || state_==INIT || state_==EXCEPT)
        //改变协程的执行函数，并将协程状态重置为INIT
        callback_=callback;
        if(getcontext(&ctx_)){
            BLUESKY_ASSERT2(false, "Fiber::reset::getcontext() error");
        }
        ctx_.uc_link = nullptr;
        ctx_.uc_stack.ss_sp = stack_;
        ctx_.uc_stack.ss_size = stacksize_;
        makecontext(&ctx_, &main_func, 0);
        state_ = INIT;
    }

    //切换到子协程执行,将子协程切进来，将主协程切出去
    //正在运行的协程一定是主协程，因此将当前协程的上下文状态保存在tMainFiber中
    //要运行子协程，则子协程此时的状态一定不为TERM或者EXEC
    //当且进来之后，将子协程的状态改为EXEC,但是不管主协程的状态是什么
    void Fiber::resume()
    {
        BLUESKY_ASSERT(state_ != TERM && state_ != EXEC);
        set_current_fiber(this);
        state_ = EXEC;
        if(swapcontext(&tMainFiber->ctx_, &ctx_))
        {
            BLUESKY_ASSERT2(false, "Fiber::swap_in::swapcontext");
        }
    }

    //将当前程序切换到后台,将主协程切进来,将子协程切出去
    void Fiber::yield()
    {
        //保存当前上下文并将上下文切换到新的上下文运行
        //如果当前协程为EXEC，那么切换出去后将其置为READY态
        //当前协程:this
        //另一个协程:tMainFiber
        BLUESKY_ASSERT(state_ == EXEC || state_ == TERM);
        if(state_!=TERM){
            state_ = READY;
        }
        set_current_fiber(tMainFiber.get());
        if(swapcontext(&ctx_,&tMainFiber->ctx_))
        {
            BLUESKY_ASSERT2(false,"Fiber::swap_out::swapcontext");
        }

    }
    
    //设置当前协程
    void Fiber::set_current_fiber(Fiber* f)
    {
        tCurFiber=f;
    }

    //返回当前协程
    Fiber::Ptr Fiber::get_current_fiber()
    {
        //如果当前协程存活，则直接返回
        //否则创建一个主协程
        if (tCurFiber)
        {
            return tCurFiber->shared_from_this();
        }
        Fiber::Ptr mainFiber(new Fiber);
        //此时tSubFiber应该指向的就是mainFiber
        BLUESKY_ASSERT(tCurFiber==mainFiber.get());
        tMainFiber = mainFiber;
        return tCurFiber->shared_from_this();
    }

    //将当前协程切换到后台，并设置为READY状态
    void Fiber::yield_to_ready()
    {
        Fiber::Ptr cur=get_current_fiber();
        cur->state_=READY;
        cur->yield();
        BLUESKY_LOG_DEBUG(BLUESKY_LOG_NAME("system"))<<"Fiber::yield_to_ready";
    }

    //将当前协程切换到后台，并设置为HOLD状态
    void Fiber::yield_to_hold()
    {
        Fiber::Ptr cur=get_current_fiber();
        cur->state_=HOLD;
        cur->yield();
        BLUESKY_LOG_DEBUG(BLUESKY_LOG_NAME("system"))<<"Fiber::yield_to_hold";
    }

    uint64_t get_total_fibers()
    {
        return sFiberCount;
    }
    
    uint64_t Fiber::get_fiberID()
    {
        if(tCurFiber){
            return tCurFiber->get_id();
        }
        return 0;

    }
    /*
    uint64_t Fiber::get_id()
    {
        return id_;
    }
    */
    void Fiber::main_func()
    {
        Fiber::Ptr cur=get_current_fiber();
        BLUESKY_ASSERT(cur);
        try{
            cur->callback_();
            cur->callback_=nullptr;
            cur->state_=TERM;
        }catch(std::exception& e){
            cur->state_=EXCEPT;
            BLUESKY_LOG_ERROR(BLUESKY_LOG_NAME("system"))<<"Fiber Except: "<<e.what()
                <<"  fiber id = "<<cur->get_fiberID()<<std::endl
                <<bluesky::backtrace_to_string();
        }catch(...){
            cur->state_=EXCEPT;
            BLUESKY_LOG_ERROR(BLUESKY_LOG_NAME("system"))<<"Fiber Except,"
                <<"  fiber id = "<<cur->get_fiberID()<<std::endl
                <<bluesky::backtrace_to_string();
        }
        //回调函数执行结束，相当于该协程应该结束了
        auto raw_ptr=cur.get();//手动将`tCurFiber`的引用计数减1
        cur.reset();
        raw_ptr->yield();//协程结束时自动yield,以回到主协程
        BLUESKY_ASSERT2(false,"never reach fiber_id = "+std::to_string(raw_ptr->get_fiberID()));
    }

    std::string Fiber::state_to_string(State state)
    {
        std::string str="";
        switch(state){
            case INIT:
                str="INIT";
                break;
            case HOLD:
                str="HOLD";
                break;
            case EXEC:
                str="EXEC";
                break;
            case TERM:
                str="TERM";
                break;
            case READY:
                str="READY";
                break;
            case EXCEPT:
                str="EXCEPT";
                break;
            default:
                str="UNKNOW";
        }
        return str;
    }
} //end of namespace
