#include "bluesky/bluesky.h"

void test_backtrace()
{
    std::string str(bluesky::backtrace_to_string(1));
    std::cout<<str<<std::endl;
}

int main()
{
    //test_backtrace();
    //BLUESKY_ASSERT(2==1);
    BLUESKY_ASSERT2(0==1,"hello BLUESKY_ASSERT2");
    return 0;
}
