#include "thread.h"
#include <pthread.h>
#include <stdlib.h>

struct Athena_Thread{
    pthread_t thread;
    void *arg;
    void (*func)(void *);
};

static void *athena_pthread_thread_wrapper(void *arg){
    struct Athena_Thread *thr = (struct Athena_Thread *)arg;
    thr->func(thr->arg);
    return NULL;
}

struct Athena_Thread *Athena_CreateThread(void (*thread_function)(void *), void *arg){
    struct Athena_Thread *thr = malloc(sizeof(struct Athena_Thread));
    thr->arg = arg;
    thr->func = thread_function;
    return thr;
}

void Athena_StartThread(struct Athena_Thread *thr){
    pthread_create(&thr->thread, NULL, athena_pthread_thread_wrapper, thr);
}

void Athena_JoinThread(struct Athena_Thread *thr){
    void *x;
    pthread_join(thr->thread, &x);
}

void Athena_DestroyThread(struct Athena_Thread *thr){
    free(thr);
}
