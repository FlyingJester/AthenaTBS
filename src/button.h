#pragma once
#include "viewport.h"

struct Athena_MessageList;
struct Athena_GameState;

/* Traditionally there the first one holds the gamestate. */ 
struct Athena_ButtonArgList{
    void *arg;
    struct Athena_ButtonArgList *next;
};

struct Athena_Button{
    int x, y;
    unsigned w, h;
    const char *text;

    struct Athena_ButtonArgList *arg;
    void (*callback)(struct Athena_ButtonArgList *, struct Athena_MessageList *);
    unsigned clicked;
};

struct Athena_ButtonList{
    struct Athena_Button button;
    struct Athena_ButtonList *next;
};

int Athena_UIDrawButtons(struct Athena_ButtonList *buttons, struct Athena_Viewport *onto);
int Athena_FreeButtonList(struct Athena_ButtonList *buttons);
int Athena_FreeButtonArgList(struct Athena_ButtonArgList *args);
struct Athena_ButtonArgList *Athena_DefaultButtonArgList(struct Athena_GameState *arg);
void Athena_CopyButtonArgList(struct Athena_ButtonArgList **to, struct Athena_ButtonArgList *from);
