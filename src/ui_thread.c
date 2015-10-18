#include "ui_thread.h"
#include "player.h"
#include "time/sleep.h"
#include "time/ticks.h"
#include "font.h"
#include "viewport.h"
#include "window_style.h"
#include <TurboJSON/value.h>
#include <TurboJSON/object.h>
#include <stdlib.h>

void Athena_UIThreadWrapper(void *that){
    Athena_UIThread(that);
}

int Athena_UIThread(struct Athena_GameState *that){

    while(Athena_UIThreadFrame(that)==0)
        Athena_MillisecondSleep(1);

    return 0;
}

static int athena_ui_process_buttons(struct Athena_ButtonList *buttons, const struct Athena_Event *event, struct Athena_MessageList *messages){
    if(!buttons){
        return 0;
    }
    else{
        if(
            (buttons->button.clicked = Athena_IsWithin(buttons->button, event->x, event->y) << 2) &&
            buttons->button.callback){

            buttons->button.callback(buttons->button.arg, messages);
        }
        return athena_ui_process_buttons(buttons->next, event, messages);
    }
}

static int athena_ui_thread_handle_event(struct Athena_GameState *that, struct Athena_Event *event, struct Athena_MessageList *messages){
    if(!Athena_GetEvent(that->ui.window, event))
        return 0;
    else{
        switch(event->type){
            case athena_click_event:
                athena_ui_process_buttons(that->ui.buttons, event, messages);
                if(that->ui.menu)
                    athena_ui_process_buttons(that->ui.menu->buttons, event, messages);
                break;
            case athena_unknown_event:
                break;
            case athena_quit_event:
                that->status = 1;
                return 1;
        }

        return athena_ui_thread_handle_event(that, event, messages);
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
    struct Athena_MessageList messages;
    messages.next = NULL;
    
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
            struct Athena_Viewport onto = {NULL, 0, 0, 0, 0};
            onto.image = &that->ui.framebuffer;
            onto.w = that->ui.framebuffer.w;
            onto.h = that->ui.framebuffer.h;
            
            Athena_UIDrawButtons(that->ui.buttons, &onto);
            if(that->ui.menu)
                Athena_DrawMenu(that->ui.menu, &onto);
        }      
        athena_do_fps_drawing(&that->ui.framebuffer);
    } /* End Drawing. */
    Athena_DrawImage(that->ui.window, 0, 0, that->ui.framebuffer.w, that->ui.framebuffer.h, 0, that->ui.framebuffer.pixels);
    Athena_FlipWindow(that->ui.window);
    
    Athena_LockMonitor(that->monitor);
    {
        struct Athena_Event event;
        athena_ui_thread_handle_event(that, &event, &messages);
    }

    {
        const int status = that->status;
        if( messages.next){
            Athena_NotifyMonitor(that->monitor);
            Athena_AppendMessageList(&that->event.msg,  messages.next);  
        }
        Athena_UnlockMonitor(that->monitor);
        return status;
    }
}

int Athena_UIClickAt(const struct Athena_UI *ui, int * /* in/out */ x, int * /* in/out */ y){
    x[0] += ui->camera_x;
    y[0] += ui->camera_y;
    return 0;
}

int Athena_UIClickAt2(const struct Athena_UI *ui, int x_in, int y_in, int *x_out, int *y_out){
    const int err = Athena_UIClickAt(ui, &x_in, &y_in);
    x_out[0] = x_in;
    y_out[0] = y_in;
    
    return err;
}

static void athena_cancel_menu_callback(void *arg, struct Athena_MessageList *messages){
    struct Athena_UI *const ui = &((struct Athena_GameState *)arg)->ui;
    if(ui->menu){
        Athena_FreeButtonList(ui->menu->buttons);
        free(ui->menu);
        ui->menu = NULL;
    }
}
static const struct Athena_Button athena_cancel_button = { 0, 0, 64, 20, "Cancel", NULL, athena_cancel_menu_callback };

static void athena_end_turn_callback(void *arg, struct Athena_MessageList *messages);

static const struct Athena_Button athena_end_turn_yes_button = { 0, 0, 64, 20, "End Turn", NULL, athena_end_turn_callback };

static void athena_open_end_turn_menu(void *arg, struct Athena_MessageList *messages){
    struct Athena_UI *const ui = &((struct Athena_GameState *)arg)->ui;
    if(ui->menu){
        Athena_FreeButtonList(ui->menu->buttons);
        free(ui->menu);
    }
    ui->menu = malloc(sizeof(struct Athena_Menu));
    ui->menu->w = 80;
    ui->menu->y = ui->framebuffer.h >> 1;
    ui->menu->x = (ui->framebuffer.w - ui->menu->w) >> 1;
    ui->menu->text = "End Turn?";

    {
        struct Athena_ButtonList * const buttons = ui->menu->buttons = malloc(sizeof(struct Athena_ButtonList)),
            * const next = buttons->next = malloc(sizeof(struct Athena_ButtonList));
        buttons->button = athena_end_turn_yes_button;
        buttons->button.arg = arg;

        next->button = athena_cancel_button;
        next->button.arg = arg;
        next->next = NULL;
    }
    Athena_OrganizeMenu(ui->menu);
}

static void athena_end_turn_callback(void *arg, struct Athena_MessageList *messages){
    if(messages->next)
        athena_end_turn_callback(arg, messages->next);
    else{
        struct Athena_MessageList * const msg = malloc(sizeof(struct Athena_MessageList)); 

        int size;

        msg->msg_text = Athena_CreateEndTurnMessage(&size);
        Turbo_Object(&msg->value, msg->msg_text, msg->msg_text + size);

        msg->next = NULL;
        
        messages->next = msg;
        
        athena_cancel_menu_callback(arg, messages);
    }
}

static struct Athena_Button end_turn_button = { 128, 0, 64, 20, "End Turn", NULL, athena_open_end_turn_menu };

void Athena_UIInit(struct Athena_GameState *state){
    state->ui.buttons = malloc(sizeof(struct Athena_ButtonList));
    
    state->ui.buttons->button = end_turn_button;
    state->ui.buttons->button.arg = state;

    state->ui.buttons->next = NULL;
}
