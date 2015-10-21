#include "thread.h"
#include "kernel/OS.h"

struct Athena_Thread{
	thread_id thread;
	void *arg;
	void (*callback)(void *);
};

static int32 athena_thread_wrapper(void *data){
	Athena_Thread *thread = static_cast<Athena_Thread *>(data);
	thread->callback(thread->arg);
	return 0;
}

struct Athena_Thread *Athena_CreateThread(void (*thread_function)(void *), void *arg){
	Athena_Thread *thread = new Athena_Thread();
	thread->arg = arg;
	thread->callback = thread_function;
	thread->thread = spawn_thread(athena_thread_wrapper, "athena_worker", B_NORMAL_PRIORITY, thread);
	return thread;
}

void Athena_StartThread(struct Athena_Thread *thread){
	resume_thread(thread->thread);
}

void Athena_JoinThread(struct Athena_Thread *thread){
	status_t i;
	wait_for_thread(thread->thread, &i);
}

void Athena_DestroyThread(struct Athena_Thread *thread){
	delete thread;
}
