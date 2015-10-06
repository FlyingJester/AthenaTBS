#pragma once

#ifdef __cplusplus
extern "C"{
#endif

struct Athena_Thread;
struct Athena_Thread *Athena_CreateThread(void (*thread_function)(void *), void *arg);
void Athena_StartThread(struct Athena_Thread *);
void Athena_JoinThread(struct Athena_Thread *);
void Athena_DestroyThread(struct Athena_Thread *);

#ifdef __cplusplus
}
#endif
