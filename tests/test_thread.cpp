#include "myserver.h"
#include <time.h>

mylog::Logger::ptr g_logger = MYLOG_LOG_ROOT();

int count = 0;
// 读写锁
myserver::RWMutex s_rwmutex;
// 互斥锁
myserver::Mutex s_mutex;

void fun1() {
    // do something
    MYLOG_LOG_INFO(g_logger) << "name: " << myserver::Thread::GetName()
                             << " this.name: " << myserver::Thread::GetThis()->getName()
                             << " id: " << mylog::GetThreadId()
                             << " this.id: " << myserver::Thread::GetThis()->getId()
                             << " this.id: " << myserver::Thread::GetThis()->getId() << std::endl;

    // 需要读写锁，避免竞争条件导致每次count的结果不一致
    for(int i=0; i<1000000; i++) {
        // 这里循环1000000次，模拟读写操作, 次数越多，竞争越激烈，结果越不一致

        // 加锁 由lockguard自动释放
        // myserver::RWMutex::WriteLock lock(s_rwmutex);
        myserver::Mutex::Lock lock(s_mutex);
        count++;
    }
}

void fun2() {
    // do something
}
int main() {
    MYLOG_LOG_INFO(g_logger) << "main thread test start";
    std::vector<myserver::Thread::ptr> threads;
    for (int i = 0; i < 10; i++) {
        myserver::Thread::ptr t(new myserver::Thread(&fun1, "name_ " + std::to_string(i)));
        threads.push_back(t);
    }

    // sleep(20);

    for (int i = 0; i < 10; i++) {
        threads[i]->join();
    }

    MYLOG_LOG_INFO(g_logger) << "main thread test end";

    MYLOG_LOG_INFO(g_logger) << "count: " << count;
    return 0;
}