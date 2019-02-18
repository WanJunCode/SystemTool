#include "Monitor.h"
#include <memory>
#include "../Log/Wlog.h"

class Monitor::Impl{
public:
    Impl()
        : ownedMutex_(new std::mutex())
        , mutex_(nullptr)
        , condInitialized_(false){
        mutex_ = ownedMutex_;
        LOG_DEBUG("IMPL INSTRUCTURE");
    }

    Impl(std::mutex *value)
        : mutex_(nullptr)
        , ownedMutex_(nullptr)
        , condInitialized_(false){
        mutex_ = value;
    }

    Impl(Monitor *monitor)
        : mutex_(nullptr)
        , ownedMutex_(nullptr)
        , condInitialized_(false){
        mutex_ = &( monitor->mutex() );
    }

    ~Impl(){
        LOG_DEBUG("IMPL DESTRUCTURE");
        delete ownedMutex_;
    }

    std::mutex& mutex(){
        // 解引用  *pointer
        return *mutex_;
    }

    void lock(){
        mutex().lock();
    }

    void unlock(){
        mutex().unlock();
    }
    
    std::cv_status waitForTime(const int64_t abstime){
        std::unique_lock<std::mutex> locker(*mutex_);
        return condition.wait_for(locker,std::chrono::milliseconds(abstime));
    }

    void waitForever() const{
        std::unique_lock<std::mutex> locker(*mutex_);
        condition.wait(locker);
    }

    void notify(){
        condition.notify_one();
    }

    void notifyAll(){
        condition.notify_all();
    }

private:
    std::mutex *ownedMutex_;
    std::mutex *mutex_;

    mutable std::condition_variable condition;
    mutable bool condInitialized_;
};

Monitor::Monitor()
    : impl_(new Monitor::Impl()){
}

Monitor::Monitor(std::mutex *value)
    : impl_(new Monitor::Impl(value)){
}

Monitor::Monitor(Monitor *monitor)
    : impl_(new Monitor::Impl(monitor)){
}

Monitor::~Monitor(){
    delete impl_;
}

std::mutex& Monitor::mutex() const{
    return impl_->mutex();
}

void Monitor::lock() const{
    impl_->lock();
}

void Monitor::unlock() const{
    impl_->unlock();
}

void Monitor::waitForever() const{
    impl_->waitForever();
}

std::cv_status Monitor::waitForTime(int64_t timeout_ms) const{
    return impl_->waitForTime(timeout_ms);
}

void Monitor::notify() const{
    impl_->notify();
}

void Monitor::notifyAll() const{
    impl_->notifyAll();
}