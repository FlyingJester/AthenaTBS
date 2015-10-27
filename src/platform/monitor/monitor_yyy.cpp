#include "monitor.h"
#include "libyyymonitor/monitor.hpp"
#include <cstdlib>

struct Athena_Monitor{
    Turbo::Monitor *monitor;
};

struct Athena_Monitor *Athena_CreateMonitor(){
    struct Athena_Monitor *monitor = (struct Athena_Monitor *)malloc(sizeof(struct Athena_Monitor));
    monitor->monitor = new Turbo::Monitor();
    return monitor;
}

void Athena_DestroyMonitor(struct Athena_Monitor *monitor){
    delete monitor->monitor;
    free(monitor);
}

void Athena_LockMonitor(struct Athena_Monitor *monitor){
    monitor->monitor->Lock();
}

void Athena_UnlockMonitor(struct Athena_Monitor *monitor){
    monitor->monitor->Unlock();
}

void Athena_WaitMonitor(struct Athena_Monitor *monitor){
    monitor->monitor->Wait();
}

void Athena_NotifyMonitor(struct Athena_Monitor *monitor){
    monitor->monitor->Notify();
}
