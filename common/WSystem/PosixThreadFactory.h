#ifndef WJ_POSIX_THREAD_FACTORY
#define WJ_POSIX_THREAD_FACTORY

#include "Thread.h"
#include "Monitor.h"

// 线程工厂： 创建新的线程
class PosixThreadFactory:public ThreadFactory{
public:
    // 构造函数
    PosixThreadFactory(bool detached = false);

    // 获得一个新的线程
    WThread *newThread(Runnable *runnable) const;

    // 获得线程工厂所在线程的 id
    WThread::thread_id getCurrentThreadId() const;

};

#endif