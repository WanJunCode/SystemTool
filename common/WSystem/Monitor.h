#ifndef WJ_MONITOR
#define WJ_MONITOR

#include "config.h"
#include <condition_variable>
#include <mutex>
#include <memory>

class Monitor{
public:
    Monitor();

    explicit Monitor(std::mutex *value);

    explicit Monitor(Monitor *monitor);

    virtual ~Monitor();

public:
    std::mutex& mutex() const;

    virtual void lock() const;

    virtual void unlock() const;

    std::cv_status waitForTime(int64_t timeout_ms) const;

    void waitForever() const;

    virtual void notify() const;

    virtual void notifyAll() const;

private:
    class Impl;
    Impl *impl_;
};

// monitor RAII
class Synchronized{
public:
    Synchronized(const Monitor *monitor)
        :locker(monitor->mutex()) {}
    
    Synchronized(const Monitor& monitor)
        :locker(monitor.mutex()) {}

private:
    std::unique_lock<std::mutex> locker;
};

#endif