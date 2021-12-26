#ifndef __MACRO_H__
#define __MACRO_H__

#include "util.h"
#include <string.h>
#include <assert.h>

#define BLUESKY_ASSERT(x) \
    if(!(x)){ \
        BLUESKY_LOG_ERROR(BLUESKY_LOG_ROOT())<<" ASSERTION: " #x \
            <<"\nbacktrace:\n" \
            <<bluesky::backtrace_to_string(100,2,"  "); \
        assert(x); \
    }

#define BLUESKY_ASSERT2(x, y) \
    if(!(x)){ \
        BLUESKY_LOG_ERROR(BLUESKY_LOG_ROOT())<<"ASSERTION: " #x \
            <<"\n"<<y\
            <<"\nbacktrace:\n" \
            <<bluesky::backtrace_to_string(100,2,"  "); \
        assert(x); \
    }
#endif
