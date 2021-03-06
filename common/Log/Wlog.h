#ifndef WANJUN_LOG_H
#define WANJUN_LOG_H

#include "../Tool/Wtool.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <mutex>
#include <pthread.h>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/CategoryStream.hh>

#define LOG_FILE "log_file.out"

#define LOG_DEBUG(fmt,...) \
    Log::getInstance().printf(pthread_self(),StripFileName(__FILE__),__LINE__,__FUNCTION__,fmt,##__VA_ARGS__);


class Log{
public:
    Log();
    ~Log();

    template<typename T>
    Log& operator << (const T&);
    void printf_w(const char *cmd, ...);
    void printf_w_notime(const char *cmd, ...);
    void printf(unsigned long pthread_id,const std::string filename,int line,const std::string function,const char *cmd,...);

    // 单例模式
    static Log& getInstance();
    static Log root_log;

private:
    std::ofstream out;
    std::mutex mutex_;
    log4cpp::Category& root;
};

template<typename T>
Log& Log::operator << (const T& data){
    out<<data<<std::flush;
    return *this;
}

#endif // !WANJUN_LOG_H