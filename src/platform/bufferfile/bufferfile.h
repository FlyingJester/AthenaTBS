#pragma once
#include "../export.h"

#ifdef __cplusplus
extern "C" {
#endif

ATHENA_PLATFORM_EXPORT void *BufferFile(const char *file, int *size);
ATHENA_PLATFORM_EXPORT void FreeBufferFile(void *in, int size);

#ifdef __cplusplus
}
#endif
