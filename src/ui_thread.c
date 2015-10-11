#include "ui_thread.h"
#include "window_style.h"
#include "time/sleep.h"
#include "time/ticks.h"
#include "font.h"

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

static void athena_do_fps_drawing(struct Athena_Image *to){/* Finally do FPS info drawing */
    static unsigned long s_tick = 0, s_ave_tick, last_fps, fs = 1000;
    float fps = Athena_GetMillisecondTicks() - s_tick;

    fps = 1000.0f / fps;

    s_ave_tick = ((float)s_ave_tick + fps) / 2.0f;
    fs++;

    if(fs >= 100){
        last_fps = s_ave_tick;
/*        printf("FPS: %i (%i, %i)\n", (int)s_ave_tick, (int)s_tick, (int)Athena_GetMillisecondTicks()); */
        fs = 0;
        s_ave_tick = fps;
    }
    
    {
        char buffer[0x80];
        sprintf(buffer, "%i", (short)last_fps);
        WriteString(GetSystemFont(), buffer, to, to->w - 64, 16);
    }
    s_tick = Athena_GetMillisecondTicks();
}/* End FPS info drawing */

int Athena_UIThreadFrame(struct Athena_GameState *that){
    struct Athena_MessageList *messages = NULL;
    
    { /* Start Drawing. Maybe someday move this out of here. Who knows. Not me. */

        { /* Field Drawing, requires a lock. */
            Athena_DrawField(that->field, &that->ui.framebuffer, that->ui.camera_x, that->ui.camera_y);

        } /* End Field Drawing */
        { /* Draw info bar */
            struct Athena_Viewport port = { NULL, 0, 0, 128, 32 };
            struct Athena_WindowStyle style;

            Athena_DefaultWindowStyle(&style);

            port.image = &that->ui.framebuffer;

            Athena_DrawWindowStyle(&style, &port);
        } /* End info bar Drawing */
        
        athena_do_fps_drawing(&that->ui.framebuffer);
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
