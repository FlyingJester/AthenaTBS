#include "ui_thread.h"
#include "player.h"
#include "time/sleep.h"
#include "time/ticks.h"
#include "font.h"
#include "viewport.h"
#include "window_style.h"
#include <stdlib.h>

void Athena_UIThreadWrapper(void *that){
    Athena_UIThread(that);
}

int Athena_UIThread(struct Athena_GameState *that){

    while(Athena_UIThreadFrame(that)==0)
        Athena_MillisecondSleep(1);

    return 0;
}

static int athena_ui_draw_buttons(struct Athena_ButtonList *buttons, struct Athena_Viewport *onto){
    if(!buttons){
        return 0;
    }
    else{
        onto->x = buttons->button.x;
        onto->y = buttons->button.y;
        onto->w = buttons->button.w;
        onto->h = buttons->button.h;
        Athena_DrawDefaultWindowStyle(onto);
        WriteString(GetSystemFont(), buttons->button.text, onto->image, buttons->button.x + 2, buttons->button.y + 2);

        if(buttons->button.clicked){
            Athena_BlendViewport(onto, Athena_RGBAToRaw(0xFF>>1, 0xFF>>1, 0xFF>>1, 0xFF), Athena_RGBARawMultiply);
        }

        buttons->button.clicked = 0;

        return athena_ui_draw_buttons(buttons->next, onto);
    }
}

static int athena_ui_process_buttons(struct Athena_ButtonList *buttons, const struct Athena_Event *event){
    if(!buttons){
        return 0;
    }
    else{
        buttons->button.clicked = Athena_IsWithin(buttons->button, event->x, event->y);
        if(buttons->button.callback && buttons->button.clicked){
            buttons->button.callback(buttons->button.arg);
        }
        return athena_ui_process_buttons(buttons->next, event);
    }
}

static int athena_ui_thread_handle_event(struct Athena_GameState *that, struct Athena_Event *event){
    if(!Athena_GetEvent(that->ui.window, event))
        return 0;
    else{
        switch(event->type){
            case athena_click_event:
                athena_ui_process_buttons(that->ui.buttons, event);
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
        printf("FPS: %i (%i, %i)\n", (int)s_ave_tick, (int)s_tick, (int)Athena_GetMillisecondTicks()); 
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
            port.image = &that->ui.framebuffer;

            Athena_DrawPlayerDataBox(that->players + that->whose_turn, &port);

/*
            struct Athena_WindowStyle style;

            Athena_DefaultWindowStyle(&style);

            port.image = &that->ui.framebuffer;

            Athena_DrawWindowStyle(&style, &port);
*/
        } /* End info bar Drawing */
        { /* Draw buttons */
            struct Athena_Viewport onto;
            onto.image = &that->ui.framebuffer;
            athena_ui_draw_buttons(that->ui.buttons, &onto);
        }      
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

/*

struct Athena_Button{
    int x, y;
    unsigned w, h;
    const char *text;

    void *arg;
    void (*callback)(void *);
};


struct Athena_ButtonList{
    struct Athena_Button button;
    struct Athena_ButtonList *next;
};

*/

static void athena_end_turn_callback(void *arg){

}

static struct Athena_Button end_turn_button = { 128, 0, 64, 20, "End Turn", NULL, athena_end_turn_callback };

void Athena_UIInit(struct Athena_UI *ui){
    ui->buttons = malloc(sizeof(struct Athena_ButtonList));
    ui->buttons->button = end_turn_button;
    ui->buttons->next = NULL;
}
