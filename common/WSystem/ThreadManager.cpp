#include <deque>
#include <mutex>
#include <set>
#include <map>
#include <condition_variable>
#include <sys/time.h>

#include "../Log/Wlog.h"
#include "ThreadManager.h"
#include "Thread.h"
#include "utime.h"

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
    // start 前必须要先设置 threadFactory_ 
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
            // 删除所有的工作线程
            removeWorkersUnderLock(workerCount_);
        }

        state_ = ThreadManager::STATE::STOPPED;
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

    std::mutex& mutex(){
        return mutex_;
    }

    // 线程池添加工作线程
    virtual void addWorker(size_t value) override;

    virtual void removeWorker(size_t value) override{
        std::lock_guard<std::mutex> locker(mutex_);
        removeWorkersUnderLock(value);
    }

    virtual size_t idleWorkCount() const override{
        return idleCount_;
    }

    virtual size_t workerCount() override{
        std::lock_guard<std::mutex> locker(mutex_);
        return workerCount_;
    }

    virtual size_t pendingTaskCount() override{
        std::lock_guard<std::mutex> locker(mutex_);
        return tasks_.size();
    }

    virtual size_t totalTaskCount() override{
        std::lock_guard<std::mutex> locker(mutex_);
        return tasks_.size() + workerCount_ - idleCount_;
    }


    virtual size_t pendingTaskCountMax() override{
        std::lock_guard<std::mutex> locker(mutex_);
        return pendingTaskCountMax_;
    }

    virtual size_t expiredTaskCount() override{
        std::lock_guard<std::mutex> locker(mutex_);
        return expiredCount_;
    }

    virtual void pendingTaskCountMax(const size_t value) {
        std::lock_guard<std::mutex> locker(mutex_);
        pendingTaskCountMax_ = value;
    }

    // 添加任务
    virtual void add(std::shared_ptr<Runnable> value,int64_t expiration) override{
        mutex_.lock();
        if( state_ != ThreadManager::STATE::STARTED){
            LOG_DEBUG("ThreadManager Impl add not STARTED");
            return;
        }

        if(pendingTaskCountMax_ > 0 && (tasks_.size() >= pendingTaskCountMax_)){
            removeExpired(true);
        }

        if(pendingTaskCountMax_ > 0 && (tasks_.size())){
            
        }


    }

    virtual void remove(std::shared_ptr<Runnable> task) override{

    }

    virtual std::shared_ptr<Runnable> removeNextPending() override{
        return nullptr;
    }

    virtual void removeExpiredTasks() override{

    }

    virtual void setExpireCallback(ExpireCallback expireCallback) override{

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
    
    void removeExpired(bool justOne){
        int64_t now = 0LL;

        for(auto it = tasks_.begin();it != tasks_.end(); ){
            if(now == 0LL){
                now = Util::currentTime();
            }

            // if((*it)->)
        }
    }

    // 判断线程工厂是否在线程池中运行
    bool canSleep() const{
        const WThread::thread_id id =threadFactory_->getCurrentThreadId();
        return idMap_.find(id) == idMap_.end();
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
    // std::timed_mutex time_mutex_;
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
    Worker(ThreadManager::Impl *value)
        : manager_(value){
    }

    virtual void run() override{

    }

private:
    ThreadManager::Impl *manager_;
    STATE state_;
};


void ThreadManager::Impl::addWorker(size_t value){
    std::set<WThread *> newThreads;
    for(size_t i = 0;i<value;++i){
        ThreadManager::Worker *worker = new ThreadManager::Worker(this);
        newThreads.insert(threadFactory_->newThread(worker));
    }

    std::unique_lock<std::mutex> locker(mutex_);
    workerMaxCount_ += value;
    workers_.insert(newThreads.begin(),newThreads.end());

    for(auto it = newThreads.begin();it!=newThreads.end();++it){
        ThreadManager::Worker *worker = dynamic_cast<ThreadManager::Worker *>((*it)->runnable());
        worker->state_ = ThreadManager::Worker::STATE::STARTING;
        (*it)->start();
        idMap_.insert(std::pair<const WThread::thread_id,WThread *>((*it)->getId(),*it));
    }

    while(workerCount_ != workerMaxCount_){
        workerMonitor_.wait(locker);
    }
}