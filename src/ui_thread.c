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
    
    { /* Start Drawing. Maybe someday move this out of here. Who knows. Not me. */

        { /* Field drawing, requires a lock. */
            Athena_DrawField(that->field, &that->ui.framebuffer, that->ui.camera_x, that->ui.camera_y);
            
        } /* End Field Drawing */
        
    } /* End Drawing. */
    
    Athena_LockMonitor(that->monitor);
    {
        const int status = that->status;
        if(messages){
            Athena_NotifyMonitor(that->monitor);
            Athena_AppendMessageList(&that->event.msg, messages);  
        }      
        Athena_UnlockMonitor(that->monitor);
        return status;
    }
}
