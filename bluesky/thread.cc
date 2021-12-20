#include "log.h"
#include "thread.h"

namespace bluesky
{
    static thread_local Thread *t_thread = nullptr;
    static thread_local std::string t_thread_name = "unknow";


    Thread::Thread(std::function<void()> cb, const std::string &name)
        : callback_(cb), name_(name)
    {
        if (name.empty())
        {
            name_ = "unknow";
        }
        int ret = pthread_create(&thread_, NULL, &Thread::run, this);
        if (ret)
        {
            BLUESKY_LOG_ERROR(BLUESKY_LOG_NAME("system")) << "Thread::pthread_create failed, ret=" << ret 
                            << " pthread name = " << name;
            throw std::logic_error("pthread_create error");
        }
        semaphore_.wait();
    }

    Thread::~Thread()
    {
        if(thread_){
            pthread_detach(thread_);
        }
    }

    void Thread::join()
    {
        if(thread_){
            int ret = pthread_join(thread_, NULL);
            if(ret){
                BLUESKY_LOG_ERROR(BLUESKY_LOG_NAME("system")) << "Thread::join()::pthread_join thread fail, ret=" << ret
                                                   << " name=" << name_;
                throw std::logic_error("Thread::join()::pthread_join error");
            }
            thread_ = 0;
        }
    }

    void* Thread::run(void* arg)
    {
        Thread *thread = (Thread *)arg;
        t_thread = thread;
        t_thread_name = thread->name_;
        thread->id_ = bluesky::get_threadID();
        pthread_setname_np(pthread_self(), thread->name_.substr(0, 15).c_str());

        std::function<void()> cb;
        cb.swap(thread->callback_);
        thread->semaphore_.post();
        cb();
        return 0;
    }

    const std::string& Thread::get_name()
    {
        return t_thread_name;
    }
    Thread* Thread::get_this()
    {
        return t_thread;
    }
 
    void Thread::set_name(const std::string& name)
    {
        if(t_thread){
            t_thread->name_ = name;
        }
        t_thread_name = name;
    }

} //end of namespace

