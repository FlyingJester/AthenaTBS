#include "thread.h"
#import <Foundation/Foundation.h>
#include <unistd.h>

@interface Athena_ThreadWrapper : NSThread

     - (void)main;

    @property(assign) void *arg;
    @property(assign) void (*thread_func)(void *);

@end

@implementation Athena_ThreadWrapper : NSThread
 - (void)main{
    _thread_func(_arg);
 }

@end

struct Athena_Thread{
    Athena_ThreadWrapper *thr;
};

struct Athena_Thread *Athena_CreateThread(void (*thread_func)(void *), void *arg){

    struct Athena_Thread *thr = malloc(sizeof(struct Athena_Thread));
    thr->thr = [[Athena_ThreadWrapper alloc] init];

    thr->thr.arg = arg;
    thr->thr.thread_func = thread_func;

    return thr;

}

void Athena_StartThread(struct Athena_Thread *thr){
    [thr->thr start];
}

void Athena_JoinThread(struct Athena_Thread *thr){
    while([thr->thr isExecuting])
        usleep(1000);
}

void Athena_DestroyThread(struct Athena_Thread *thr){
    [thr->thr release];
    free(thr);
}
