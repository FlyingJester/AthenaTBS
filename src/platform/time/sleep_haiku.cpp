#include <kernel/OS.h>

extern "C"
void Athena_MillisecondSleep(unsigned ms){
    snooze_until(ms*1000, B_SYSTEM_TIMEBASE);
}
