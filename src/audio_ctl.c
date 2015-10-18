#include "audio_ctl.h"
#include <stdlib.h>

static struct Athena_SoundContext *system_ctx = NULL;

struct Athena_SoundContext *Athena_GetSystemSoundContext(){
    if(!system_ctx)
        system_ctx = Athena_CreateSoundContext();
    return system_ctx;
}
