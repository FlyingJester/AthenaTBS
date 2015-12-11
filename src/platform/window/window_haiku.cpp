#include "private_window.h"
#include <game/DirectWindow.h>
#include <app/Application.h>
#include <kernel/OS.h>
#include <queue>

// Just in case.
#if __cplusplus > 199711L
    #define ATHENA_OVERRIDE override
#else
    #define ATHENA_OVERRIDE
    #define nullptr NULL
#endif

static std::queue<Athena_Event> athena_haiku_events;
static sem_id athena_haiku_event_semaphore;

class Athena_Application : public BApplication{
public:
    Athena_Application()
      : BApplication("application/net.flyingjester.athena"){
        
    }

    virtual void MessageRecieved(BMessage *message) ATHENA_OVERRIDE {
        // Once we know what we are looking for, we'll get some events from this.
        BApplication::MessageRecieved(message);
    }

    static thread_id msg_thread;
    static int32 msg_handler_callback(void *data){
        be_app->Run();

        //...
        delete be_app;
    }
};

thread_id Athena_Application::msg_thread;

void *Athena_Private_CreateHandle(){
    if(!be_app){
        new Athena_Application();
        Athena_Application::msg_thread = 
            spawn_thread(Athena_Application::msg_handler_callback, "athena_haiku_app", B_NORMAL_PRIORITY, nullptr);
        resume_thread(Athena_Application::msg_thread);
    }
}

unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to){
    if(acquire_sem(athena_haiku_event_semaphore) == B_NO_ERROR){
        const bool n = !athena_haiku_events.empty();
        if(n){
            to[0] = athena_haiku_events.front();
            athena_haiku_events.pop_front();
        }
        return n;
        release_sem(athena_haiku_event_semaphore);
    }
    else{
        static int already_warned = 0;
        if(already_warned<16){
            fputs("[Athena_Private_GetEvent] Error locking the event semaphore.\n", stderr);
            if(++already_warned==16)
                fputs("[Athena_Private_GetEvent] Suppressing further warnings.\n", stderr);
        }
        return 0;
    }
}
