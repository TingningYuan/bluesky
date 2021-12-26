#include "bluesky/bluesky.h"

void callback1()
{
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT())<<"run in fiber callback1 begin()";
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT())<<"run in fiber callback1 end()"; 
}

void callback2()
{
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT())<<"run in fiber callback2 begin()";
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT())<<"run in fiber callback2 end()"; 
}

int main()
{
    bluesky::Fiber::get_current_fiber();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT())<<"main begin";
    bluesky::Fiber::Ptr fiber(new bluesky::Fiber(callback1));
    fiber->resume();
    //fiber->yield();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "main after resume";
    bluesky::Fiber::Ptr fiber2(new bluesky::Fiber(callback1));
    fiber2->resume();
    fiber2->reset(callback2);
    fiber2->resume();
    BLUESKY_LOG_INFO(BLUESKY_LOG_ROOT()) << "fiber2 over";
    return 0;
}
