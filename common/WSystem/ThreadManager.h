#ifndef WANJUN_THREAD_MANAGER
#define WANJUN_THREAD_MANAGER

#include <memory>
#include <functional>

class Runnable;
class ThreadFactory;

// 虚基类
class ThreadManager{
public:
    enum class STATE{ UNINITIALIZED, STARTING, STARTED, JOINING, STOPPING, STOPPED};

    using ExpireCallback = std::function<void(std::shared_ptr<Runnable>)>;

public:
    virtual ~ThreadManager() {}

public:
    virtual void start() = 0;

    virtual void stop() = 0;

    virtual ThreadManager::STATE state() const = 0;

    virtual std::shared_ptr<ThreadFactory> threadFactory() = 0;

    virtual void threadFactory(std::shared_ptr<ThreadFactory> value) = 0;

    virtual void addWorker(size_t value = 1) = 0;

    virtual void removeWorker(size_t value = 1) = 0;

    virtual size_t idleWorkCount() const = 0;

    virtual size_t workerCount() = 0;

    virtual size_t pendingTaskCount() = 0;

    virtual size_t totalTaskCount() = 0;

    virtual size_t pendingTaskCountMax() = 0;

    virtual size_t expiredTaskCount() = 0;

    virtual void add(std::shared_ptr<Runnable> task, int64_t expiration = 0LL) = 0;

    virtual void try_add(std::shared_ptr<Runnable> task, int64_t expiration = 0LL) = 0;

    virtual void remove(std::shared_ptr<Runnable> task) = 0;

    virtual std::shared_ptr<Runnable> removeNextPending() = 0;

    virtual void removeExpiredTasks() = 0;

    virtual void setExpireCallback(ExpireCallback expireCallback) = 0;

protected:
    ThreadManager() {};

public:
    class Task;     // 任务类
    class Worker;   // 工作线程类
    class Impl;     // 线程池实现类
};

#endif