#include "util.h"
#include "log.h"
#include "fiber.h"

namespace bluesky
{
    pid_t get_threadID()
    {
        return syscall(SYS_gettid);
    }

    uint32_t get_fiberID()
    {
        return bluesky::Fiber::get_fiberID();
    }
    
    void get_backtrace(std::vector<std::string>& bt, int size, int skip)
    {
        int j, nptrs;
        void **buffer=(void**)malloc(sizeof(void*)*size);
        char **strings;

        nptrs=backtrace(buffer, size);
        strings=backtrace_symbols(buffer, nptrs);
        if(!strings){
            BLUESKY_LOG_ERROR(BLUESKY_LOG_NAME("system"))<<"backtrace_symbols error";
            free(buffer);
            return;
        }
        for(j=skip;j<nptrs;j++){
            bt.push_back(strings[j]);
        }
        free(buffer);
        free(strings);

    }
    std::string backtrace_to_string(int size, int skip, std::string prefix)
    {
        std::vector<std::string> bt;
        get_backtrace(bt,size, skip);
        std::stringstream ss;
        for(size_t i=0;i<bt.size();i++){
            ss<<prefix<<bt[i]<<std::endl;
        }
        return ss.str();
    }
} //end of namespace
