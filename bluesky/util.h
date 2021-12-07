#ifndef __UTIL_H__
#define __UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdint.h>

namespace bluesky{

    pid_t get_threadID();

    uint32_t get_fiberID();

} //end of namespace



#endif