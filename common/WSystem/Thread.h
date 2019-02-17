#ifndef WJ_THREAD_MANAGER
#define WJ_THREAD_MANAGER

#include <thread>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
// #include <pthread.h>

class WThread;
class Runnable{
public:
    virtual ~Runnable() {}

public:
    virtual void run() = 0;

    virtual WThread *thread(){
        return thread_;
    }

    virtual void Thread(WThread *value){
        thread_ = value;   
    }
private:
    WThread *thread_;
};

// 线程基类
class WThread{
public:
    using thread_id=std::thread::id;

public:
    virtual ~WThread() {}

    virtual void start() = 0;

    virtual void join() = 0;

    virtual thread_id getId() = 0;

    virtual Runnable *runnable() const{
        return runnable_;
    }

public:
    static bool is_current(thread_id id){
        return std::this_thread::get_id()==id;
    }

    static thread_id get_current(){
        return std::this_thread::get_id();
    }

protected:
    virtual void runnable(Runnable *value){
        runnable_ = value;
    }

private:
    Runnable *runnable_;
};

// 线程工厂基类
class ThreadFactory{
public:
    virtual ~ThreadFactory() {}

public:
    inline bool isDetached() const{
        return detached_;
    }

    inline void setDetached(bool detached){
        detached_ = detached;
    }

public:
    virtual WThread* newThread(Runnable *runnable) const = 0;

    virtual WThread::thread_id getCurrentThreadId() const = 0;

public:
    static const WThread::thread_id unknow_thread_id;

protected:
    ThreadFactory(bool detached)
        :detached_(detached) {}

private:
    bool detached_;
};

#endif