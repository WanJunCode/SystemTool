#include <stdio.h>
#include "../common/Log/Wlog.h"
#include "../common/WSystem/Monitor.h"
int main(int argc,char* argv[]){
    LOG_DEBUG("TEST FOR COMMON LOG");
    Monitor monitor;
    Synchronized sync(monitor);
    return 0;
}
