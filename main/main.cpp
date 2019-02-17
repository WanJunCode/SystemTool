#include <stdio.h>
#include "../common/Log/Wlog.h"
#include "../common/WSystem/Monitor.h"
int main(int argc,char* argv[]){
    LOG_DEBUG("TEST FOR COMMON LOG");
    Monitor *monitor = new Monitor();
    Synchronized sync(monitor);
    delete monitor;
    return 0;
}
