#include "button.h"
#include "image.h"
#include "window_style.h"
#include "font.h"
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
