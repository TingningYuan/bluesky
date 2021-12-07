#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <iostream>
#include <pthread.h>
#include <boost/noncopyable.hpp>

namespace bluesky
{
    template <class T>
    class Singleton : boost::noncopyable
    {
    public:
        static T& get_instance()
        {
            pthread_once(&ponce_, &Singleton::init);
            return *value_;
        }

    private:
        Singleton();
        ~Singleton();

        static void init()
        {
            value_ = new T();
        }

    private:
        static pthread_once_t ponce_;
        static T* value_;
    };

    template <class T>
    pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

    template <class T>
    T* Singleton<T>::value_ = nullptr;
} //end of namespace
#endif
