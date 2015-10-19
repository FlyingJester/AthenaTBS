#pragma once
#include "audio/audio.h"

struct Athena_Sound *Athena_LoadOpusFile(const char *path);
struct Athena_Sound *Athena_LoadOpusFileForContext(const char *path, struct Athena_SoundContext *ctx);
struct Athena_Sound *Athena_LoadOpusMemory(const void *data, int size, struct Athena_SoundContext *ctx);
int Athena_LoadOpusSoundMemory(const void *data, int size, struct Athena_Sound *to, struct Athena_SoundContext *ctx);
