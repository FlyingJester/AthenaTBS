#include "ticks.h"
#include <sys/time.h>
#include <stdlib.h>

unsigned long Athena_GetMillisecondTicks(){
    struct timeval t;
    gettimeofday(&t, NULL);
    {
        unsigned long time_ms = t.tv_sec * 1000;
        time_ms += t.tv_usec / 1000;
        return time_ms;
    }
}
