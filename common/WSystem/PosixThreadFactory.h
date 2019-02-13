#ifndef WJ_POSIX_THREAD_FACTORY
#define WJ_POSIX_THREAD_FACTORY

#include "Thread.h"
#include "Monitor.h"

class PosixThreadFactory:public ThreadFactory{
public:
    enum class POLICY { OTHER, FIFO, ROUND_ROBIN };

    enum class PRIORITY{
        LOWEST = 0,
        LOWER = 1,
        LOW = 2,
        NORMAL = 3,
        HIGH = 4,
        HIGHER = 5,
        HIGHEST = 6,
        INCREMENT = 7,
        DECREMENT = 8
    };

    // 构造函数
    PosixThreadFactory(POLICY policy = POLICY::ROUND_ROBIN,
                        PRIORITY priority = PRIORITY::NORMAL,
                        int stackSize = 1,
                        bool detached = false);

    PosixThreadFactory(bool detached);

    WThread *newThread(Runnable *runnable) const;

    // 获得线程工厂所在线程的 id
    WThread::thread_id getCurrentThreadId() const;

    virtual int getStackSize() const;

    virtual void setStackSize(int value);

    virtual PRIORITY getPriority() const;

    virtual void setPriority(PRIORITY priority);
    
private:
    POLICY policy_;
    PRIORITY priority_;
    int stackSize_;
};

#endif