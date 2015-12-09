#include "ui_thread.h"
#include "player.h"
#include "time/sleep.h"
#include "time/ticks.h"
#include "load_opus.h"
#include "audio/audio.h"
#include "font.h"
#include "viewport.h"
#include "window_style.h"
#include <TurboJSON/value.h>
#include <TurboJSON/object.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void unit_build_selection_callback(struct Athena_ButtonArgList *args, struct Athena_MessageList *messages){
    struct Athena_Unit *const unit = Athena_FindTypeInArgList(args, "source_unit");
    struct Athena_BuildPosition *const build = Athena_FindTypeInArgList(args, "build");
    if(build && unit){
        int size;

        struct Athena_MessageList * const msg = malloc(sizeof(struct Athena_MessageList)); 
        
        msg->msg_text = Athena_CreateBuildMessage(&size, unit, build->clazz, build->x, build->y);
        
        Turbo_Object(&msg->value, msg->msg_text, msg->msg_text + size);

        msg->next = NULL;

        Athena_AppendMessageList(&(messages->next), msg);

#ifndef NDEBUG
        printf("[unit_build_selection_callback]Building unit %s using %s at %i, %i\n", build->clazz->name, unit->clazz->name, build->x, build->y);
#endif
    }

    if(build)
        free(build);

}

static void unit_movement_selection_callback(struct Athena_ButtonArgList *args, struct Athena_MessageList *messages){
    struct Athena_SelectingPosition *const position = Athena_FindTypeInArgList(args, "destination");
    struct Athena_Unit *const unit = Athena_FindTypeInArgList(args, "source_unit");
    if(position && unit){
        int size;

        struct Athena_MessageList * const msg = malloc(sizeof(struct Athena_MessageList)); 
        
        msg->msg_text = Athena_CreateMovementMessage(&size, unit, position->x, position->y);
        
        Turbo_Object(&msg->value, msg->msg_text, msg->msg_text + size);

        msg->next = NULL;

        Athena_AppendMessageList(&(messages->next), msg);

#ifndef NDEBUG
        printf("[unit_movement_selection_callback]Moving unit %s to %i, %i\n", unit->clazz->name, position->x, position->y);
#endif
    }

    if(position)
        free(position);

}

void unit_attack_selection_callback(struct Athena_ButtonArgList *args, struct Athena_MessageList *messages){
    struct Athena_SelectingPosition *const position = Athena_FindTypeInArgList(args, "destination");
    struct Athena_Unit *const unit = Athena_FindTypeInArgList(args, "source_unit");
    if(position && unit && position->unit){
        int size;

        struct Athena_MessageList * const msg = malloc(sizeof(struct Athena_MessageList)); 
        
        msg->msg_text = Athena_CreateAttackMessage(&size, unit, position->x, position->y);
        
        Turbo_Object(&msg->value, msg->msg_text, msg->msg_text + size);

        msg->next = NULL;

        Athena_AppendMessageList(&(messages->next), msg);

#ifndef NDEBUG
        printf("[unit_attack_selection_callback]Attacking unit %s at %i, %i, using unit %s at %i, %i\n", 
            unit->clazz->name, unit->x, unit->y, position->unit->clazz->name, position->x, position->y);
#endif        
        
    }

    if(position)
        free(position);
}

void unit_movement_callback(struct Athena_ButtonArgList *args, struct Athena_MessageList *messages){
    if(!args)
        return;
    else{
        struct Athena_GameState *const state = args->arg;

        if(state->ui.selection_arg)
            Athena_FreeButtonArgList(state->ui.selection_arg);

        Athena_CopyButtonArgList(&state->ui.selection_arg, args);
        
        state->ui.selection_callback = unit_movement_selection_callback;

        if(state->ui.positions_arg)
            Athena_FreeButtonArgList(state->ui.positions_arg);

        Athena_CopyButtonArgList(&state->ui.positions_arg, args);
        state->ui.positions_callback = Athena_MovementPositions;

    }
    Athena_CancelMenuCallback(args, messages);
}

void unit_build_callback(struct Athena_ButtonArgList *args, struct Athena_MessageList *messages){
    if(!args)
        return;
    else{
/*        struct Athena_GameState *const state = args->arg; */
        
    }
}

void unit_attack_callback(struct Athena_ButtonArgList *args, struct Athena_MessageList *messages){
    if(!args)
        return;
    else{
        struct Athena_GameState *const state = args->arg;

        if(state->ui.selection_arg)
            Athena_FreeButtonArgList(state->ui.selection_arg);

        Athena_CopyButtonArgList(&state->ui.selection_arg, args);

        state->ui.selection_callback = unit_attack_selection_callback;

        if(state->ui.positions_arg)
            Athena_FreeButtonArgList(state->ui.positions_arg);

        Athena_CopyButtonArgList(&state->ui.positions_arg, args);
        state->ui.positions_callback = Athena_AttackRangePositions;

    }
    Athena_CancelMenuCallback(args, messages);
}

static const struct Athena_Button athena_move_button = { 0, 0, 64, 20, "Move", NULL, unit_movement_callback };
static const struct Athena_Button athena_attack_button = { 0, 0, 64, 20, "Attack", NULL, unit_attack_callback };
static const struct Athena_Button athena_build_button = { 0, 0, 64, 20, "Build Unit", NULL, Athena_CancelMenuCallback };

static struct Athena_Menu *athena_generate_unit_menu(struct Athena_GameState *arg, struct Athena_Unit *unit){
    struct Athena_Menu *unit_menu = malloc(sizeof(struct Athena_Menu));
    struct Athena_ButtonList *buttons = NULL;

    if(unit->actions){
        if(unit->clazz->is_building){
            struct Athena_Button *const first_button = Athena_AppendButton(&buttons, athena_build_button);
            first_button->arg = Athena_DefaultButtonArgList(arg);
            Athena_AppendButtonArgList(first_button->arg, unit, "source_unit");
        }
        else{
            struct Athena_Button *const first_button = Athena_AppendButton(&buttons, athena_attack_button);
            first_button->arg = Athena_DefaultButtonArgList(arg);
            Athena_AppendButtonArgList(first_button->arg, unit, "source_unit");

            /* Push 'next' ahead one to make room for movement. */
            if(unit->movement){
                struct Athena_Button *const movement_button = Athena_AppendButton(&buttons, athena_move_button);
                movement_button->arg = Athena_DefaultButtonArgList(arg);
                Athena_AppendButtonArgList(movement_button->arg, unit, "source_unit");
            }

        }
    }
    
    Athena_AppendButton(&buttons, athena_cancel_button)->arg = Athena_DefaultButtonArgList(arg);

    unit_menu->buttons = buttons;

    if(unit->clazz->is_building){    
        unit_menu->w = 120;   
        unit_menu->text = "Building Action";
        
    }
    else{
        unit_menu->w = 100;
        unit_menu->text = "Unit Action";
    }

    return unit_menu;
}

void Athena_UIThreadWrapper(void *that){
    Athena_UIThread(that);
}

int Athena_UIThread(struct Athena_GameState *that){

    while(Athena_UIThreadFrame(that)==0)
        Athena_MillisecondSleep(1);

    return 0;
}

static int athena_process_selector(const struct Athena_Field *field, struct Athena_UI *ui, const struct Athena_Event *event, struct Athena_MessageList *messages){
    if(!ui->selection_callback)
        return 0;
    else{
        struct Athena_SelectingPosition *position = malloc(sizeof(struct Athena_SelectingPosition));
        struct Athena_PositionList *list = ui->positions_callback(ui->positions_arg);
        
        if(list){

            int x, y;
            Athena_FieldPixelXYToTileXY(field, event->x, event->y, &x, &y, ui->camera_x, ui->camera_y);

            if(Athena_PositionInList(list, x, y)){

                position->unit = Athena_FindUnitAt(field->units, position->x = x, position->y = y);
                Athena_AppendButtonArgList(ui->selection_arg, position, "destination");

                ui->selection_callback(ui->selection_arg, messages);

            }
            Athena_FreePositionList(list);
        }

        ui->selection_callback = NULL;
        if(ui->selection_arg)
            Athena_FreeButtonArgList(ui->selection_arg);
        ui->selection_arg = NULL;

        ui->positions_callback = NULL;
        if(ui->positions_arg)
            Athena_FreeButtonArgList(ui->positions_arg);
        ui->positions_arg = NULL;
        
        return 1;
    }
}

static int athena_test_unit_index(struct Athena_UnitList *units, struct Athena_Unit *unit, int i){
    if(!units)
        return -1;
    else if(unit == &units->unit)
        return i;
    else
        return athena_test_unit_index(units->next, unit, i+1);
}

static int athena_ui_get_unit_menu(struct Athena_GameState *that, struct Athena_UnitList *units, const struct Athena_Event *event, struct Athena_MessageList *messages){
    if(!units)
        return 0;
    else{
        int x, y;
        Athena_FieldPixelXYToTileXY(that->field, event->x, event->y, &x, &y, that->ui.camera_x, that->ui.camera_y);
        
        {
            struct Athena_Unit *const unit = Athena_FindUnitAt(units, x, y);
            const int i = athena_test_unit_index(units, unit, 0);

#ifndef NDEBUG
            printf("[athena_ui_get_unit_menu]Selected unit %i\n", i);
#endif

            if(unit && unit->owner && unit->owner->human && unit->owner == that->players + that->whose_turn){
                {
                    struct Athena_ButtonArgList arg_list = {NULL, NULL};
                    arg_list.arg = that;
                    Athena_CancelMenuCallback(&arg_list, messages);
                }
                that->ui.menu = athena_generate_unit_menu(that, unit);
                that->ui.menu->x = event->x; that->ui.menu->y = event->y;
                Athena_OrganizeMenu(that->ui.menu);
                return 1;
            }
        }
        
        return athena_ui_get_unit_menu(that, units->next, event, messages);
    }
}

static int athena_ui_thread_handle_event(struct Athena_GameState *that, struct Athena_Event *event, struct Athena_MessageList *messages){
    if(!Athena_GetEvent(that->ui.window, event))
        return 0;
    else{
        switch(event->type){
            case athena_click_event:
                if(event->which == athena_left_mouse_button || event->which == athena_unknown_mouse_button){
                    if(Athena_ProcessButtons(that, that->ui.buttons, event, messages))
                        break;

                    if(that->ui.menu && Athena_ProcessButtons(that, that->ui.menu->buttons, event, messages))
                        break;

                    if(athena_process_selector(that->field, &that->ui, event, messages))
                        break;

                    if(athena_ui_get_unit_menu(that, that->field->units, event, messages))
                        break;

                }
                else if(event->which == athena_right_mouse_button){
                    {
                        struct Athena_ButtonArgList arg_list = {NULL, NULL};
                        arg_list.arg = that;
                        Athena_CancelMenuCallback(&arg_list, messages);

                        if(that->ui.selection_arg)
                            Athena_FreeButtonArgList(that->ui.selection_arg);
                        that->ui.selection_arg = NULL;
                        that->ui.selection_callback = NULL;

                        if(that->ui.positions_arg)
                            Athena_FreeButtonArgList(that->ui.positions_arg);
                        that->ui.positions_arg = NULL;
                        that->ui.positions_callback = NULL;
                    }
                }
                break;
            case athena_unknown_event:
            case athena_key_event:
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
#ifndef NDEBUG
/*        printf("FPS: %i (%i, %i)\n", (int)s_ave_tick, (int)s_tick, (int)Athena_GetMillisecondTicks()); */
#endif
        fs = 0;
        s_ave_tick = fps;
    }
    
    {
        char buffer[0x80];
        sprintf(buffer, "%i", (short)last_fps);
        WriteString(GetSystemFont(), buffer, to, to->w - 64, 16);
    }
    s_tick = Athena_GetMillisecondTicks();
}

static void athena_draw_selector(const struct Athena_Field *field, struct Athena_UI *ui){
    if(ui->selection_callback || Athena_IsKeyPressed(ui->window, 'o')){
        int x, y, mouse_x, mouse_y;
        Athena_GetMousePosition(ui->window, &mouse_x, &mouse_y);

        Athena_FieldPixelXYToTileXY(field, mouse_x, mouse_y, &x, &y, ui->camera_x, ui->camera_y);
        Athena_FieldTileXYToPixelXY(field, x, y, &x, &y, ui->camera_x, ui->camera_y);
        
        Athena_BlendRect(&ui->framebuffer, x, y, field->tileset->tile_width, field->tileset->tile_height, Athena_RGBAToRaw(0xE0, 0xE0, 0x30, 0xFF), Athena_RGBARawAverage);
    }
}

static void athena_positions_callback(void *arg, int x, int y){
    struct Athena_GameState *state = arg;
    Athena_FieldTileXYToPixelXY(state->field, x, y, &x, &y, state->ui.camera_x, state->ui.camera_y);
    
    Athena_BlendRect(&state->ui.framebuffer, x, y, state->field->tileset->tile_width, state->field->tileset->tile_height,
        Athena_RGBAToRaw(0x30, 0x30, 0x10, 0), Athena_RGBARawAdd);
}

int Athena_UIThreadFrame(struct Athena_GameState *that){
    struct Athena_MessageList messages;
    messages.next = NULL;
/*
    ...
*/
    memset(that->ui.framebuffer.pixels, 0, that->ui.framebuffer.w * that->ui.framebuffer.h * 4);

    { /* Camera controls. This must be handled before anything else, so that drawing can reflect changes as soon as possible. */
        int64_t amount = 1;
        const int64_t current_time = Athena_GetMillisecondTicks();
        if(that->ui.last_camera_motion.type==ui_cam_key && that->ui.last_camera_motion.time)
            amount = current_time - that->ui.last_camera_motion.time;
        if(amount){
            if(Athena_IsKeyPressed(that->ui.window, 'w')){
                that->ui.camera_y-=amount;
                that->ui.last_camera_motion.type=ui_cam_key;
                that->ui.last_camera_motion.time=current_time;
            }
            else if(Athena_IsKeyPressed(that->ui.window, 'a')){
                that->ui.camera_x-=amount;
                that->ui.last_camera_motion.type=ui_cam_key;
                that->ui.last_camera_motion.time=current_time;
            }
            else if(Athena_IsKeyPressed(that->ui.window, 's')){
                that->ui.camera_y+=amount;
                that->ui.last_camera_motion.type=ui_cam_key;
                that->ui.last_camera_motion.time=current_time;
            }
            else if(Athena_IsKeyPressed(that->ui.window, 'd')){
                that->ui.camera_x+=amount;
                that->ui.last_camera_motion.type=ui_cam_key;
                that->ui.last_camera_motion.time=current_time;
            }
            else if(that->ui.last_camera_motion.type==ui_cam_key){
                that->ui.last_camera_motion.type=ui_cam_none;
            }
        }
/*
        if(that->ui.camera_x<-(that->ui.framebuffer.w>>1))
            that->ui.camera_x = -(that->ui.framebuffer.w>>1);
            
        if(that->ui.camera_y<-(that->ui.framebuffer.h>>1))
            that->ui.camera_y = -(that->ui.framebuffer.h>>1);
*/
    }

    { /* Start Drawing. Maybe someday move this out of here. Who knows. Not me. */
        
        Athena_LockMonitor(that->monitor);

        { /* Field Drawing, requires a lock. */
            Athena_DrawField(that->field, &that->ui.framebuffer, that->ui.camera_x, that->ui.camera_y);
            
            Athena_DrawUnits(that->field->units, &that->ui.framebuffer, 
                that->field->tileset->tile_width, that->field->tileset->tile_height, that->ui.camera_x, that->ui.camera_y);
            
            Athena_DrawUnitsHealthBars(that->field->units, &that->ui.framebuffer, 
                that->field->tileset->tile_width, that->field->tileset->tile_height, that->ui.camera_x, that->ui.camera_y);

        } /* End Field Drawing */
        { /* Selector... */            
            athena_draw_selector(that->field, &that->ui);

            if(that->ui.positions_callback){
                struct Athena_PositionList *pos = that->ui.positions_callback(that->ui.positions_arg);
                Athena_FoldPositions(pos, athena_positions_callback, that);
            }
        }
        { /* Draw info bar */

            struct Athena_Viewport port = { NULL, 0, 0, 160, 64 };
            port.image = &that->ui.framebuffer;

            Athena_DrawPlayerDataBox(that->players + that->whose_turn, &port);

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

        Athena_UnlockMonitor(that->monitor);        

        athena_do_fps_drawing(&that->ui.framebuffer);
    } /* End Drawing. */

    Athena_Update(that->ui.window, 0, that->ui.framebuffer.pixels);
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

/* Generally all menu callbacks will first call this to ensure that any existing menu is taken down properly. */
void Athena_CancelMenuCallback(struct Athena_ButtonArgList *arg, struct Athena_MessageList *messages){
    struct Athena_UI *const ui = &((struct Athena_GameState *)(arg->arg))->ui;
    if(ui->menu){
        Athena_FreeButtonList(ui->menu->buttons);
        free(ui->menu);
        ui->menu = NULL;
    }
}

const struct Athena_Button athena_cancel_button = { 0, 0, 64, 20, "Cancel", NULL, Athena_CancelMenuCallback };

static void athena_end_turn_callback(struct Athena_ButtonArgList *arg, struct Athena_MessageList *messages);

static const struct Athena_Button athena_end_turn_yes_button = { 0, 0, 64, 20, "End Turn", NULL, athena_end_turn_callback };

static void athena_open_end_turn_menu(struct Athena_ButtonArgList *arg, struct Athena_MessageList *messages){
    struct Athena_UI *const ui = &((struct Athena_GameState *)(arg->arg))->ui;

    Athena_CancelMenuCallback(arg, messages);

    ui->menu = malloc(sizeof(struct Athena_Menu));
    ui->menu->w = 80;
    
    ui->menu->y = ui->framebuffer.h >> 1;
    ui->menu->x = (ui->framebuffer.w - ui->menu->w) >> 1;

    ui->menu->text = "End Turn?";

    {
        struct Athena_ButtonList * const buttons = ui->menu->buttons = malloc(sizeof(struct Athena_ButtonList)),
            * const next = buttons->next = malloc(sizeof(struct Athena_ButtonList));
        buttons->button = athena_end_turn_yes_button;
        buttons->button.arg = Athena_DefaultButtonArgList(arg->arg);

        next->button = athena_cancel_button;
        next->button.arg = Athena_DefaultButtonArgList(arg->arg);
        next->next = NULL;
    }
    Athena_OrganizeMenu(ui->menu);
}

static void athena_end_turn_callback(struct Athena_ButtonArgList *arg, struct Athena_MessageList *messages){
    if(messages->next)
        athena_end_turn_callback(arg, messages->next);
    else{
        struct Athena_MessageList * const msg = malloc(sizeof(struct Athena_MessageList)); 

        int size;

        msg->msg_text = Athena_CreateEndTurnMessage(&size);
        Turbo_Object(&msg->value, msg->msg_text, msg->msg_text + size);
        
        msg->next = NULL;
        
        Athena_AppendMessageList(&(messages->next), msg);

        Athena_CancelMenuCallback(arg, messages);
    }
}

static struct Athena_Button end_turn_button = { 160, 0, 0, 20, "End Turn", NULL, athena_open_end_turn_menu };

void Athena_UIInit(struct Athena_GameState *state){
    state->ui.click_sound = Athena_LoadOpusFile("res/sounds/bloop.opus");
    
    {
        struct Athena_SoundConfig config;
        Athena_SoundGetConfig(state->ui.click_sound, &config);
        config.volume = 0.10;
        Athena_SoundSetConfig(state->ui.click_sound, &config);
    }

    state->ui.buttons = malloc(sizeof(struct Athena_ButtonList));
    
    if(end_turn_button.w==0){
        end_turn_button.w = StringWidth(GetSystemFont(), end_turn_button.text) + 8;
    }

    state->ui.buttons->button = end_turn_button;
    state->ui.buttons->button.arg = Athena_DefaultButtonArgList(state);

    state->ui.buttons->next = NULL;
}
