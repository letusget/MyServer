#include "util.h"

namespace myserver
{
/**
 * 这里不使用pthread_self返回的线程ID是因为pthread_self返回的是pthread库的线程id，
 * 不同进程间的线程id可能相同，这里使用syscall返回的是Linux下的线程id
*/
pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

u_int32_t GetFiberId(){
    return 0;
}
} // namespace myserver
