#include "button.h"
#include "image.h"
#include "window_style.h"
#include "font.h"
#include "game.h"
#include <stdlib.h>
#include <string.h>

int Athena_UIDrawButtons(struct Athena_ButtonList *buttons, struct Athena_Viewport *onto){
    if(!buttons){
        return 0;
    }
    else{
        onto->x = buttons->button.x;
        onto->y = buttons->button.y;
        onto->w = buttons->button.w;
        onto->h = buttons->button.h;
        Athena_DrawDefaultWindowStyle(onto);
        WriteString(GetSystemFont(), buttons->button.text, onto->image, buttons->button.x + 4, buttons->button.y + 4);

        if(buttons->button.clicked){
            buttons->button.clicked--;
            Athena_BlendViewport(onto, Athena_RGBAToRaw(0xFF>>1, 0xFF>>1, 0xFF>>1, 0xFF), Athena_RGBARawMultiply);
        }

        return Athena_UIDrawButtons(buttons->next, onto);
    }
}

int Athena_FreeButtonList(struct Athena_ButtonList *buttons){
    if(!buttons)
        return 0;
    else {
        struct Athena_ButtonList *next = buttons->next;
        Athena_FreeButtonArgList(buttons->button.arg);
        free(buttons);
        return Athena_FreeButtonList(next);
    }
}

int Athena_FreeButtonArgList(struct Athena_ButtonArgList *args){
    if(!args)
        return 0;
    else {
        struct Athena_ButtonArgList *arg = args->next;
        free(args);
        return Athena_FreeButtonArgList(arg);
    }
}

struct Athena_ButtonArgList *Athena_DefaultButtonArgList(struct Athena_GameState *arg){
    struct Athena_ButtonArgList *const arg_list = malloc(sizeof(struct Athena_ButtonArgList));
    arg_list->arg = arg;
    arg_list->next = NULL;
    arg_list->type = "game_state";
    return arg_list;
}

int Athena_AppendButtonArgList(struct Athena_ButtonArgList *list, void *arg, const char *type){
    if(list->next)
        return Athena_AppendButtonArgList(list->next, arg, type);
    else{
        list->next = malloc(sizeof(struct Athena_ButtonArgList));
        list->next->arg = arg;
        list->next->type = type;
        list->next->next = NULL;
        return 0;
    }
}

void Athena_CopyButtonArgList(struct Athena_ButtonArgList **to, struct Athena_ButtonArgList *from){
    if(!from)
        to[0] = NULL;
    else{
        to[0] = malloc(sizeof(struct Athena_ButtonArgList));
        to[0]->type= from->type;
        to[0]->arg = from->arg;
        Athena_CopyButtonArgList(&(to[0]->next), from->next);
    }
}

void *Athena_FindTypeInArgList(struct Athena_ButtonArgList *list, const char *type){
    if(!list)
        return NULL;
    else if(strcmp(type, list->type)==0)
        return list->arg;
    else
        return Athena_FindTypeInArgList(list->next, type);
}

struct Athena_Button *Athena_AppendButtonList(struct Athena_ButtonList **to, struct Athena_ButtonList *button){
    if(to[0]==NULL){
        to[0] = button;
        return &to[0]->button;
    }
    else
        return Athena_AppendButtonList(&(to[0]->next), button);
}

struct Athena_Button *Athena_AppendButton(struct Athena_ButtonList **to, struct Athena_Button button){
    struct Athena_ButtonList *const b = malloc(sizeof(struct Athena_ButtonList));
    b->button = button;
    b->next = NULL;
    return Athena_AppendButtonList(to, b);
}

static int athena_ui_process_buttons_iter(struct Athena_GameState *that, struct Athena_ButtonList *buttons, const struct Athena_Event *event, struct Athena_MessageList *messages){
    if(!buttons){
        return 0;
    }
    else{
        if(
            (buttons->button.clicked = Athena_IsWithin(buttons->button, event->x, event->y) << 2) &&
            buttons->button.callback){
            if(that->ui.click_sound)
                Athena_SoundPlay(that->ui.click_sound);
            buttons->button.callback(buttons->button.arg, messages);
            return 1;
        }
        return athena_ui_process_buttons_iter(that, buttons->next, event, messages);
    }
}

int Athena_ProcessButtons(struct Athena_GameState *that, struct Athena_ButtonList *buttons, const struct Athena_Event *event, struct Athena_MessageList *messages){
    return athena_ui_process_buttons_iter(that, buttons, event, messages);
}
