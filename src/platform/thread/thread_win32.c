#include "thread.h"
#include <windows.h>

struct Athena_Thread{
	void *arg;
	void (*func)(void *);
	HANDLE thread;
};

static DWORD WINAPI athena_win32_thread_wrapper(LPVOID arg){
	struct Athena_Thread * const thr = arg;
	thr->func(thr->arg);
	return 0;
}

struct Athena_Thread *Athena_CreateThread(void (*thread_function)(void *), void *arg){
	struct Athena_Thread * const thr = malloc(sizeof(struct Athena_Thread));
	thr->arg = arg;
	thr->func = thread_function;
	thr->thread = NULL;
	return thr;
}

void Athena_StartThread(struct Athena_Thread *thr){
	thr->thread = CreateThread(NULL, 0, athena_win32_thread_wrapper, thr, 0, CREATE_SUSPENDED);
}

void Athena_JoinThread(struct Athena_Thread *thr){
	WaitForSingleObject(thr->thread, INFINITE);
}

void Athena_DestroyThread(struct Athena_Thread *thr){
	free(thr);
}

