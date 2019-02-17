#include <deque>
#include <mutex>
#include <set>
#include <map>
#include <condition_variable>
#include "../Log/Wlog.h"
#include "ThreadManager.h"
#include "Thread.h"

// 线程池 实现类
class ThreadManager::Impl : public ThreadManager{
    friend class ThreadManager::Task;
    friend class ThreadManager::Worker;
    using TaskQueue = std::deque< std::shared_ptr< ThreadManager::Task > >;
public:
    Impl()
        : workerCount_(0)
        , workerMaxCount_(0)
        , idleCount_(0)
        , pendingTaskCountMax_(0)
        , expiredCount_(0)
        , state_(ThreadManager::STATE::UNINITIALIZED)
        , threadFactory_(nullptr){
    }

    ~Impl(){
        stop();
    }

public:
    virtual void start() override{
        std::unique_lock<std::mutex> locker(mutex_);
        if(state_ == ThreadManager::STATE::STOPPED){
            return;
        }
        if(state_ == ThreadManager::STATE::UNINITIALIZED){
            if(!threadFactory_){
                LOG_DEBUG("thread factory is nullptr\n");
                return;
            }
            state_ = ThreadManager::STATE::STARTED;
            monitor_.notify_all();
        }

        while(state_ == STATE::STARTING){
            monitor_.wait(locker);
        }
    }

    virtual void stop() override{
        std::unique_lock<std::mutex> locker(mutex_);
        bool doStop = false;

        if(state_ != ThreadManager::STATE::STOPPING &&
            state_ != ThreadManager::STATE::JOINING &&
            state_ != ThreadManager::STATE::STOPPED){
            doStop = true;
            state_ = ThreadManager::STATE::JOINING;
        }

        if(doStop){
            removeWorkersUnderLock(workerCount_);
        }

        state_ = ThreadManager::STATE::STOPPED;
    }

private:
    void removeWorkersUnderLock(size_t value){
        if(value > workerMaxCount_){
            LOG_DEBUG("Invalid Argument");
            return;
        }

        workerMaxCount_ -= value;

        if(idleCount_ > value){
            for(size_t ix = 0;ix< value; ++ix){
                monitor_.notify_one();
            }
        }else{
            monitor_.notify_all();
        }

        // 等待 workercount 符合标准
        while(workerCount_ != workerMaxCount_){
            std::unique_lock<std::mutex> locker(mutex_);
            workerMonitor_.wait(locker);
        }

        for(auto ix = deadWorkers_.begin();ix != deadWorkers_.end();++ix){
            // join 所有的线程，等待线程结束
            if(threadFactory_->isDetached() == false){
                (*ix)->join();
            }

            idMap_.erase((*ix)->getId());
            workers_.erase(*ix);
        }

        deadWorkers_.clear();
    }

    virtual ThreadManager::STATE state() const override{
        return state_;
    }

    virtual std::shared_ptr<ThreadFactory> threadFactory() override{
        std::lock_guard<std::mutex> locker(mutex_);
        return threadFactory_;
    }

    virtual void threadFactory(std::shared_ptr<ThreadFactory> value) override{
        std::lock_guard<std::mutex> locker(mutex_);
        if(threadFactory_ && threadFactory_->isDetached() != value->isDetached()){
            LOG_DEBUG("thread is invalid argument.");
            return;
        }
        threadFactory_ = value;
    }

    // 线程池添加工作线程
    virtual void addWorker(size_t value) override{
        std::set<WThread *> newThreads;
        for(size_t i = 0;i<value;++i){
            // ThreadManager::Worker *worker = new ThreadManager::Worker(this);
            // newThreads.insert(threadFactory_->newThread(worker));
        }
    }

private:
    size_t workerCount_;
    size_t workerMaxCount_;
    size_t idleCount_;
    size_t pendingTaskCountMax_;
    size_t expiredCount_;
    ExpireCallback expireCallback;  // 超时回调函数

    ThreadManager::STATE state_;
    std::shared_ptr<ThreadFactory> threadFactory_;  // 线程工厂
    TaskQueue tasks_;
    std::mutex mutex_;
    std::condition_variable monitor_;
    std::condition_variable maxMonitor_;
    std::condition_variable workerMonitor_;

    std::set<WThread *> workers_;
    std::set<WThread *> deadWorkers_;
    std::map<const WThread::thread_id, WThread *> idMap_;
};

class ThreadManager::Task : public Runnable{

};

class ThreadManager::Worker : public Runnable{
    friend class ThreadManager::Impl;
    enum class STATE{ UNINITIALIZED, STARTING, STARTED, STOPPING, STOPPED };

public:
    Worker(ThreadManager::Impl *manager)
        :manager_(manager){
    }

private:
    ThreadManager::Impl *manager_;
    STATE state_;
};