#include "ui_thread.h"
#include "time/sleep.h"

void Athena_UIThreadWrapper(void *that){
    Athena_UIThread(that);
}

int Athena_UIThread(struct Athena_GameState *that){

    while(Athena_UIThreadFrame(that)==0)
        Athena_MillisecondSleep(1);

    return 0;
}

int athena_ui_thread_handle_event(struct Athena_GameState *that, struct Athena_Event *event){
    if(!Athena_GetEvent(that->ui.window, event))
        return 0;
    else{
        switch(event->type){
            case athena_click_event:
                break;
            case athena_unknown_event:
                break;
            case athena_quit_event:
                that->status = 1;
                return 1;
        }

        return athena_ui_thread_handle_event(that, event);
    }
}

int Athena_UIThreadFrame(struct Athena_GameState *that){
    struct Athena_MessageList *messages = NULL;
    
    { /* Start Drawing. Maybe someday move this out of here. Who knows. Not me. */

        { /* Field drawing, requires a lock. */
            Athena_DrawField(that->field, &that->ui.framebuffer, that->ui.camera_x, that->ui.camera_y);

        } /* End Field Drawing */
        
    } /* End Drawing. */
    Athena_DrawImage(that->ui.window, 0, 0, that->ui.framebuffer.w, that->ui.framebuffer.h, 0, that->ui.framebuffer.pixels);
    Athena_FlipWindow(that->ui.window);
    
    Athena_LockMonitor(that->monitor);
    {
        struct Athena_Event event;
        athena_ui_thread_handle_event(that, &event);
    }

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
