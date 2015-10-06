#include "ui_thread.h"
#include "time/sleep.h"

void Athena_UIThreadWrapper(void *that){
    Athena_UIThread(that);
}

int Athena_UIThread(struct Athena_GameState *that){

    while(Athena_UIThreadFrame(that) == 0)
        Athena_MillisecondSleep(1);

    return 0;
}

int Athena_UIThreadFrame(struct Athena_GameState *that){
    struct Athena_MessageList *messages = NULL;
    Athena_LockMonitor(that->event.monitor);
    
    { /* Start Drawing. Maybe someday move this out of here. Who knows. Not me. */
    
    
    } /* End Drawing. */
    
    {
        const int status = that->event.status;
        if(messages){
            Athena_NotifyMonitor(that->event.monitor);
            Athena_AppendMessageList(&that->event.msg, messages);  
        }      
        Athena_UnlockMonitor(that->event.monitor);
        return status;
    }
}
