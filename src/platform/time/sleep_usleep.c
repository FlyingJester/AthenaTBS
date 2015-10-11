#include "sleep.h"
#include <unistd.h>

void Athena_MillisecondSleep(unsigned ms){
    usleep(ms * 1000);
}
