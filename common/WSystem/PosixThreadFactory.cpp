#include "PosixThreadFactory.h"
#include "../Log/Wlog.h"
#include <mutex>
#include <condition_variable>

// 线程池创建的线程
class PthreadThread : public WThread{
public:
    // 当前 thread 的状态
    enum class STATE { uninitialized, starting, started, stopping, stopped };

    static void threadMain(void *arg);

public:
    PthreadThread(bool detached,Runnable *runnable)
        : threadId_(0)
        , thread_(nullptr)
        , state_(STATE::uninitialized)
        , self_(this)
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

    virtual WThread::thread_id getId() override{
        return threadId_;
    }

    virtual void start() override{
        if(getState() != STATE::uninitialized){
            return;
        }

        setState(STATE::starting);

        std::unique_lock<std::mutex> locker(mutex_);

        thread_ = new std::thread(threadMain, this);
        if(detached_){
            // 如果设置了线程 detached
            thread_->detach();
        }

        // 等待线程的执行
        condition_.wait(locker);
    }

    virtual void join() override{
        if(!detached_ && getState() != STATE::uninitialized){
            if(thread_->joinable())
                thread_->join();
        }
    }
    
    void setThreadID(WThread::thread_id id){
        threadId_ = id;
    }

    PthreadThread::STATE getState() {
        std::lock_guard<std::mutex> locker(mutex_);
        return state_;
    }

    void setState(PthreadThread::STATE newState){
        std::lock_guard<std::mutex> locker(mutex_);
        state_ = newState;

        if(newState == STATE::started){
            condition_.notify_one();
        }
    }

    virtual Runnable *runnable() const{
        return WThread::runnable();
    }

    virtual void runnable(Runnable *value){
        WThread::runnable(value);
    }

    // 判断传入的参数 self 是否为 this
    void weakRef(PthreadThread *self){
        assert(self == this);
        self_ = self;
    }

private:
    WThread::thread_id threadId_;
    std::thread *thread_;
    PthreadThread::STATE state_;
    PthreadThread *self_;
    bool detached_;
    std::mutex mutex_;
    std::condition_variable condition_;
};

void PthreadThread::threadMain(void *self){
    PthreadThread *thread = reinterpret_cast<PthreadThread *>(self);
    // 设置 thread id
    thread->setThreadID( WThread::get_current());

    // 设置当前 状态为 started, notify_one()
    thread->setState(STATE::started);

    thread->runnable()->run();

    PthreadThread::STATE _s = thread->getState();
    if(_s != STATE::stopping && _s != STATE::stopped){
        thread->setState(STATE::stopping);
    }
}

// =============================线程工厂=====================================
PosixThreadFactory::PosixThreadFactory(bool detached)
    :ThreadFactory(detached){
}

// 返回的 WThread 调用 start() 函数开始运行
WThread *PosixThreadFactory::newThread(Runnable *runnable) const{
    return new PthreadThread(ThreadFactory::isDetached(),runnable);
}

WThread::thread_id PosixThreadFactory::getCurrentThreadId() const{
    return WThread::get_current();
}