#include "ticks.h"
#include <windows.h>

unsigned long Athena_GetMillisecondTicks(){
    return GetTickCount();
}
