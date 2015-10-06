#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct Athena_Monitor;

struct Athena_Monitor *Athena_CreateMonitor();
void Athena_DestroyMonitor(struct Athena_Monitor *);

void Athena_LockMonitor(struct Athena_Monitor *);
void Athena_UnlockMonitor(struct Athena_Monitor *);

/* The Monitor MUST be locked before Wait is called. */
void Athena_WaitMonitor(struct Athena_Monitor *);
/* The Monitor MUST be unlocked before Notify is called. */
void Athena_NotifyMonitor(struct Athena_Monitor *);

#ifdef __cplusplus
}
#endif
