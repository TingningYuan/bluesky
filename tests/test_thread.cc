#include "bluesky/thread.h"
#include "bluesky/locker.h"
#include "bluesky/log.h"
#include "bluesky/config.h"

#include <iostream>
#include <functional>
#include <time.h>
#include <yaml-cpp/yaml.h>

bluesky::Mutex g_mutex;
int count = 0;
void test_mutex()
{
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "   name: " << bluesky::Thread::get_name()
                                         << "   this.name: " << bluesky::Thread::get_this()
                                         << "   id: " << bluesky::get_threadID()
                                         << "   this.id: " << bluesky::Thread::get_this()->get_threadID();
    for (int i = 0; i < 10;i++){
        //bluesky::MutexGuard lock(g_mutex);
        bluesky::Mutex::Lock lock(g_mutex);
        ++count;
        std::cout << i << std::endl;
    }
}
void func1()
{
    for (int i = 0; i < 10;i++){
        std::cout << "hello thread" << std::endl;
    }
}

void func2()
{
    while(true)
    {
        BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "xxxxxxxxxxxxxxxxxxxxxx";
    }
}
void func3()
{
    while(true)
    {
        BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "BBBBBBBBBBBBBBBBBBBBBB";
    }
}

void test_thread()
{
    std::function<void()> cb = func1;
    bluesky::Thread thread(cb, "test_thread");
    sleep(1);
    std::cout << "thread name=" << thread.get_name() << std::endl;
    std::cout << "thread id=" << thread.get_threadID() << std::endl;
    thread.set_name("thread_thread_set_name");
    std::cout << "thread name=" << thread.get_name() << std::endl;
    std::cout << thread.get_this()->get_name() << std::endl;
    thread.join();

    //bluesky::Thread::Ptr thread2(new bluesky::Thread(cb, "test_thread2"));
    //thread2->join();
}

void test_lock()
{
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "lock test start";
    YAML::Node node = YAML::LoadFile("./conf/log.yml");
    bluesky::Config::load_from_yaml(node);

    std::vector<bluesky::Thread::Ptr> threads;
    for (int i = 0; i < 2;i++){
        bluesky::Thread::Ptr thread_1(new bluesky::Thread(&test_mutex,"thread_1"));
        bluesky::Thread::Ptr thread_2(new bluesky::Thread(&test_mutex,"thread_2"));
        threads.push_back(thread_1);
        threads.push_back(thread_2);
    }
    for (int i = 0; i < threads.size();i++){
        threads[i]->join();
    }
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "test lock end";
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << " count=" << count;
}

void test_log()
{
    std::vector<bluesky::Thread::Ptr> threads;
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "start test log";
    for (int i = 0; i < 2; i++)
    {
        bluesky::Thread::Ptr thread_1(new bluesky::Thread(&func2,"thread_1"));
        bluesky::Thread::Ptr thread_2(new bluesky::Thread(&func3,"thread_2"));
        threads.push_back(thread_1);
        threads.push_back(thread_2);
    }
    for (int i = 0; i < threads.size();i++){
        threads[i]->join();
    }
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "test log end";
}
int main()
{
    //test_lock();
    test_log();
    return 0;
}