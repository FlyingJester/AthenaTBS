#pragma once
#include "../export.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Athena_Monitor;

ATHENA_PLATFORM_EXPORT struct Athena_Monitor *Athena_CreateMonitor();
ATHENA_PLATFORM_EXPORT void Athena_DestroyMonitor(struct Athena_Monitor *);

ATHENA_PLATFORM_EXPORT void Athena_LockMonitor(struct Athena_Monitor *);
ATHENA_PLATFORM_EXPORT void Athena_UnlockMonitor(struct Athena_Monitor *);

/* The Monitor MUST be locked before Wait is called. */
ATHENA_PLATFORM_EXPORT void Athena_WaitMonitor(struct Athena_Monitor *);
/* The Monitor MUST be unlocked before Notify is called. */
ATHENA_PLATFORM_EXPORT void Athena_NotifyMonitor(struct Athena_Monitor *);

#ifdef __cplusplus
}
#endif
