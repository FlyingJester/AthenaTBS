#pragma once
#include "viewport.h"

struct Athena_MessageList;
struct Athena_GameState;

/* Traditionally there the first one holds the gamestate. */ 

/*
 * Well-defined argument types:
 *    Name: source_unit
 *    Type: Athena_Unit *
 *    Free: caller
 *
 *    Name: destination
 *    Type: Athena_SelectingPosition *
 *    Free: callee
 *
 *    Name: game_state
 *    Type: Athena_GameState *
 *    Free: caller
 *    Note: This is always the first arg (if any args exist at all)
 *
 */
struct Athena_ButtonArgList{
    void *arg;
    const char *type;
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

/* type will NOT be freed (to allow literal constants more easily) */
int Athena_AppendButtonArgList(struct Athena_ButtonArgList *list, void *arg, const char *type);
void *Athena_FindTypeInArgList(struct Athena_ButtonArgList *list, const char *type);
void Athena_CopyButtonArgList(struct Athena_ButtonArgList **to, struct Athena_ButtonArgList *from);
