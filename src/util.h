#ifndef __MYLOG_UTIL_H__
#define __MYLOG_UTIL_H__

#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>

namespace mylog
{
// 获取系统中线程ID
pid_t GetThreadId();

// 获取协程ID
u_int32_t GetFiberId();

} // namespace mylog


#endif