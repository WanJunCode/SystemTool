#include <stdio.h>
#include <memory>
#include <iostream>
#include <unistd.h>

#include "../common/Log/Wlog.h"
#include "../common/System/ThreadManager.h"
#include "../common/System/PosixThreadFactory.h"
#include "../common/System/Thread.h"

class test_task : public Runnable{
public:
    test_task(std::string message, short seconds)
        : message_(message)
        , seconds_(seconds){
        LOG_DEBUG("构造函数");
    }
    ~test_task(){
        LOG_DEBUG("析构函数");
    }
    virtual void run() override{
        for(int i=0;i<10;++i){
            sleep(seconds_);
            LOG_DEBUG("test [%s]",message_.data());
        }
    }
private:
    std::string message_;
    short seconds_;
};

int main(int argc,char* argv[]){
    LOG_DEBUG("TEST FOR COMMON LOG");
    auto manager = ThreadManager::blockingTaskThreadManager(100);
    manager->threadFactory(std::make_shared<PosixThreadFactory>(false));
    manager->start();

    manager->add(std::make_shared<test_task>("wan",2));
    manager->add(std::make_shared<test_task>("jun",1));

    LOG_DEBUG("after add test task");
    delete manager;
    return 0;
}
