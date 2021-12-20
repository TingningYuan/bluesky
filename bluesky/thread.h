#ifndef __THREAD_H__
#define __THREAD_H__

#include <memory>
#include <string>
#include <functional>
#include <pthread.h>
#include "locker.h"

namespace bluesky
{
    
/*线程类
 *成员变量：
 *      线程名
 *      线程ID
 *      线程标识符
 *      线程所要执行的函数
 *成员函数：
 *      构造函数：传入线程入口函数和线程名
 *      析构函数：
 *      得到线程名
 *      具体任务执行函数
*/
    class Thread
    {
    public:
        typedef std::shared_ptr<Thread> Ptr;

        Thread(std::function<void()> cb, const std::string &name);
        ~Thread();

        std::string get_threadname() const { return name_; }
        pid_t get_threadID() const { return id_; }
        void join();

        static Thread *get_this();
        static const std::string &get_name();
        static void set_name(const std::string &name);

    private:
        Thread(const Thread &) = delete;
        Thread(const Thread &&) = delete;
        Thread &operator==(const Thread &) = delete;

        static void *run(void *arg);

    private:
        std::string name_;
        pid_t id_ = -1;
        pthread_t thread_ = 0;
        std::function<void()> callback_;
        Semaphore semaphore_;
    };

} //end of namesapce
#endif