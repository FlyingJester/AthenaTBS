#include "sleep.h"
#include <time.h>

void Athena_MillisecondSleep(unsigned ms){
    struct timespec t;
    unsigned long ns = ms * 1000000;
    t.tv_sec = ms / 1000;
    ns -= (t.tv_sec * 1000000000);
    
    nanosleep(&t, NULL);
}
