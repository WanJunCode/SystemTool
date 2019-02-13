#include "PosixThreadFactory.h"
#include "../Log/Wlog.h"
#include <mutex>
#include <condition_variable>

// 线程池创建的线程
class PthreadThread : public WThread{
public:
    enum class STATE { uninitialized, starting, started, stopping, stopped };

    static const int MB = 1024 * 1024;

    static void *threadMain(void *arg);

private:
    WThread::thread_id id_;
    STATE state_;
    int policy_;
    int priority_;
    int stackSize_;
    PthreadThread *self_;
    bool detached_;
    // Monitor monitor_;
    std::mutex mutex_;
    std::condition_variable condition_;

public:
    PthreadThread(int policy,int priority,int stackSize,bool detached,Runnable *runnable)
        : id_(0)
        , state_(STATE::uninitialized)
        , policy_(policy)
        , priority_(priority)
        , stackSize_(stackSize)
        , detached_(detached){
        // 调用 基类 protected 函数
        this->WThread::runnable(runnable);
    }

    ~PthreadThread(){
        if(!detached_){
            try{
                join();
            }catch(...){
                // swallow everything
                LOG_DEBUG("WARNING PThreadThread join error");
            }
        }
    }

    STATE getState() {
        std::lock_guard<std::mutex> locker(mutex_);
        return state_;
    }

    void setState(STATE newState){
        std::lock_guard<std::mutex> locker(mutex_);
        state_ = newState;

        if(newState == STATE::started){
            condition_.notify_one();
        }
    }


};