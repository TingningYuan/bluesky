#include "util.h"

namespace bluesky
{
    pid_t get_threadID()
    {
        return syscall(SYS_gettid);
    }

    uint32_t get_fiberID()
    {
        return 0;
    }
} //end of namespace