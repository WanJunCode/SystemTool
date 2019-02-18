#ifndef WJ_MUTEX
#define WJ_MUTEX

#include <mutex>
#include "config.h"

// std::mutex mutex_;
// lock() unlock() try_lock()

// std::timed_mutex
// lock() unlock() try_lock_for() try_lock_until()

// std::timed_mutex capsulate
class Time_mutex{
public:
    // 单位 milliseconds
    bool try_lock_for(int milliseconds){
        auto ts = std::chrono::microseconds(milliseconds);
        return time_mutex_.try_lock_for(ts);
    }

    bool try_lock_until(int milliseconds){
        auto now = std::chrono::steady_clock::now();
        return time_mutex_.try_lock_until(now + std::chrono::milliseconds(milliseconds));
    }

private:
    std::timed_mutex time_mutex_;
};


//  pthread_rwlock_t 封装;
class ReadWriteMutex {
public:
    ReadWriteMutex() {
        CHECK_RETURN_VALUE(pthread_rwlock_init(&lock_, NULL));
    };

    virtual ~ReadWriteMutex() {
        CHECK_RETURN_VALUE(pthread_rwlock_destroy(&lock_));
    }

public:
    // these get the lock and block until it is done successfully
    void acquireRead() {
        CHECK_RETURN_VALUE(pthread_rwlock_rdlock(&lock_));
    }

    void acquireWrite() {
        CHECK_RETURN_VALUE(pthread_rwlock_wrlock(&lock_));
    }

    // these attempt to get the lock, returning false immediately if they fail
    bool attemptRead() {
        CHECK_RETURN_VALUE(pthread_rwlock_tryrdlock(&lock_));
    }

    bool attemptWrite() {
        CHECK_RETURN_VALUE(pthread_rwlock_trywrlock(&lock_));
    }

    // this releases both read and write locks
    void release() {
        CHECK_RETURN_VALUE(pthread_rwlock_unlock(&lock_));
    }

//Attibute
private:
    pthread_rwlock_t lock_;
};


// Can be used as second argument to RWGuard to make code more readable
// as to whether we're doing acquireRead() or acquireWrite().
enum RWGuardType { RW_READ = 0, RW_WRITE = 1 };

class RWGuard {
public:
    RWGuard(ReadWriteMutex& value, RWGuardType type = RW_READ)
        : rw_mutex_(value) {
        if (type == RW_WRITE) {
            rw_mutex_.acquireWrite();
        } else {
            rw_mutex_.acquireRead();
        }
    }
    ~RWGuard() {
        rw_mutex_.release();
    }

private:
    ReadWriteMutex& rw_mutex_;
};

#endif