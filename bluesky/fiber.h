#ifndef __FIBER_H__
#define __FIBER_H__

#include "thread.h"
#include <ucontext.h>
#include <functional>
#include <memory>

namespace bluesky
{
    class Fiber: public std::enable_shared_from_this<Fiber>
    {
        friend class Scheduler;
        public:
            typedef std::shared_ptr<Fiber> Ptr;

            enum State
            {
                INIT,
                HOLD,
                EXEC,
                TERM,
                READY,
                EXCEPT
            };
        public:
            Fiber(std::function<void()> callback, size_t stacksize=0);
            ~Fiber();
            
            //重置协程函数，并重置状态
            //INIT，TERM
            void reset(std::function<void()> callback);

            //切换到当前协程执行
            void resume();

            //切换到后台执行
            void yield();

            uint64_t get_id() { return id_; }

            std::string state_to_string(State state);
        public:
            //设置当前协程
            static void set_current_fiber(Fiber* f);
            //返回当前协程
            static Fiber::Ptr get_current_fiber();
            //协程切换到后台，并且设置为ready状态
            static void yield_to_ready();
            //协程切换到后台，并且设置为hold状态
            static void yield_to_hold();
            //总协程数
            static uint64_t get_total_fibers();
            
            static uint64_t get_fiberID();
                
            static void main_func();
        private:
            Fiber();

        private:
            //协程ID
            uint64_t id_=0;
            //协程栈大小
            uint32_t stacksize_=0;
            //协程状态
            State state_=INIT;
            //协程上下文
            ucontext_t ctx_;
            //协程栈地址
            void* stack_=nullptr;
            //协程入口函数
            std::function<void()> callback_;
    };

} //end of namespace


#endif
