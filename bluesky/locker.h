#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <atomic>
#include <thread>
#include <memory>
#include <boost/noncopyable.hpp>

namespace bluesky
{

    /* 封装信号量类 */
    class Semaphore : boost::noncopyable
    {
    public:
        //创建信号量
        Semaphore()
        {
            if (sem_init(&sem_, 0, 0) != 0)
            {
                throw std::logic_error("Semaphore::sem_init error");
            }
        }

        //销毁信号量
        ~Semaphore()
        {
            sem_destroy(&sem_);
        }

        //等待信号量
        void wait()
        {
            if (sem_wait(&sem_))
            {
                throw std::logic_error("Semaphore::sem_wait error");
            }
        }

        //增加信号量
        void post()
        {
            if (sem_post(&sem_))
            {
                throw std::logic_error("Semaphore::sem_post error");
            }
        }

    private:
        sem_t sem_;
    };

    template <class T>
    struct ScopedLockImpl : boost::noncopyable
    {
    public:
        ScopedLockImpl(T &mutex) : mutex_(mutex)
        {
            mutex_.lock();
            locked_ = true;
        }
        ~ScopedLockImpl()

        {
            if (locked_)
            {
                mutex_.unlock();
                locked_ = false;
            }
        }
        void lock()
        {
            if (!locked_)
            {
                mutex_.lock();
                locked_ = true;
            }
        }
        void unlock()
        {
            if (locked_)
            {
                mutex_.lock();
                locked_ = false;
            }
        }

    private:
        T &mutex_;
        bool locked_;
    };

    template <class T>
    struct ReadScopedLockImpl : boost::noncopyable
    {
    public:
        ReadScopedLockImpl(T &mutex) : mutex_(mutex)
        {
            mutex.rdlock();
            locked_ = true;
        }
        ~ReadScopedLockImpl()
        {
            unlock();
        }
        void lock()
        {
            if (!locked_)
            {
                mutex_.rdlock();
                locked_ = true;
            }
        }
        void unlock()
        {
            if (!locked_)
            {
                mutex_.unlock();
                locked_ = false;
            }
        }

    private:
        T &mutex_;
        bool locked_;
    };

    template <class T>
    struct WriteScopedLockImpl
    {
    public:
        WriteScopedLockImpl(T &mutex) : mutex_(mutex)
        {
            mutex.rdlock();
            locked_ = true;
        }
        ~WriteScopedLockImpl()
        {
            unlock();
        }
        void lock()
        {
            if (!locked_)
            {
                mutex_.rdlock();
                locked_ = true;
            }
        }
        void unlock()
        {
            if (!locked_)
            {
                mutex_.unlock();
                locked_ = false;
            }
        }

    private:
        T &mutex_;
        bool locked_;
    };

    /* 封装互斥量类 */
    class Mutex : boost::noncopyable
    {
    public:
        typedef ScopedLockImpl<Mutex> Lock;

        Mutex()
        {
            if (pthread_mutex_init(&mutex_, NULL) != 0)
            {
                throw std::logic_error("Mutex::pthread_mutex_init error");
            }
        }
        ~Mutex()
        {
            pthread_mutex_destroy(&mutex_);
        }

        void lock()
        {
            if (pthread_mutex_lock(&mutex_))
            {
                throw std::logic_error("Mutex::pthread_mutex_lock error");
            }
        }

        void unlock()
        {
            if (pthread_mutex_unlock(&mutex_))
            {
                throw std::logic_error("Mutex::pthread_mutex_unlock error");
            }
        }

    private:
        pthread_mutex_t mutex_;
    };

    class NullMutex
    {
    public:
        typedef ScopedLockImpl<NullMutex> Lock;
        NullMutex() {}
        ~NullMutex() {}
        void lock() {}
        void unlock() {}
    };

    class MutexGuard : boost::noncopyable
    {
    public:
        MutexGuard(Mutex &mutex) : mutex_(mutex)
        {
            mutex_.lock();
        }
        ~MutexGuard()
        {
            mutex_.unlock();
        }

    private:
        Mutex &mutex_;
    };

    class RWMutex : boost::noncopyable
    {
    public:
        typedef ReadScopedLockImpl<RWMutex> ReadLock;
        typedef WriteScopedLockImpl<RWMutex> WriteLock;

        RWMutex()
        {
            pthread_rwlock_init(&lock_, NULL);
        }
        ~RWMutex()
        {
            pthread_rwlock_destroy(&lock_);
        }
        void rdlock()
        {
            pthread_rwlock_rdlock(&lock_);
        }
        void wrlock()
        {
            pthread_rwlock_wrlock(&lock_);
        }
        void unlock()
        {
            pthread_rwlock_unlock(&lock_);
        }

    private:
        pthread_rwlock_t lock_;
    };

    class NullRWMutex
    {
    public:
        typedef ReadScopedLockImpl<NullMutex> ReadLock;
        typedef WriteScopedLockImpl<NullMutex> WriteLock;

        NullRWMutex() {}
        ~NullRWMutex() {}

        void rdlock() {}
        void wrlock() {}
        void unlock() {}
    };

    /* 封装条件变量类 */
    class Cond : boost::noncopyable
    {
    public:
        Cond()
        {
            if (pthread_mutex_init(&mutex_, NULL))
            {
                throw std::logic_error("Cond::pthread_mutex_init error");
            }
            if (pthread_cond_init(&cond_, NULL))
            {
                throw std::logic_error("Cond::pthread_cond_init error");
            }
        }
        ~Cond()
        {
            pthread_mutex_destroy(&mutex_);
            pthread_cond_destroy(&cond_);
        }

        void wait()
        {
            int ret = 0;
            pthread_mutex_lock(&mutex_);
            ret = pthread_cond_wait(&cond_, &mutex_);
            pthread_mutex_unlock(&mutex_);
            if (ret != 0)
            {
                throw std::logic_error("Cond::wait error");
            }
        }

        void signal()
        {
            if (pthread_cond_signal(&cond_))
            {
                throw std::logic_error("Cond::pthread_cond_signal error");
            }
        }

    private:
        pthread_cond_t cond_;
        pthread_mutex_t mutex_;
    };

} //end of namespace
#endif
