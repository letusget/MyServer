#ifndef __MYSERVER_MACRO_H__
#define __MYSERVER_MACRO_H__

#include <assert.h>
#include <string.h>

#include "util.h"

#define MYSERVER_ASSERT(x)                                                                                  \
    if (!(x)) {                                                                                             \
        MYLOG_LOG_ERROR(MYLOG_LOG_ROOT())                                                                   \
            << "Assertion failed: " << #x << "\n backtrace: " << myserver::BacktraceToString(100, 2, "\t"); \
        assert(x);                                                                                          \
    }

#define MYSERVER_ASSERT_MSG(x, msg)                                                                          \
    if (!(x)) {                                                                                              \
        MYLOG_LOG_ERROR(MYLOG_LOG_ROOT()) << "Assertion failed: " << #x << " (" << msg                       \
                                          << ")\n backtrace: " << myserver::BacktraceToString(100, 2, "\t"); \
        assert(x);                                                                                           \
    }

#endif  // __MYSERVER_MACRO_H__