#ifndef __UTIL_H__
#define __UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <execinfo.h>
#include <string>
#include <vector>

namespace bluesky{

    pid_t get_threadID();

    uint32_t get_fiberID();
    
    void get_backtrace(std::vector<std::string>& bt, int size, int skip); 
    
    std::string backtrace_to_string(int size=10, int skip=0, std::string prefix=" ");

} //end of namespace



#endif
