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
 *    Name: build
 *    Type: Athena_BuildPosition *
 *    Free: callee
 *    Note: This is used to indicate where and what to be built by a source_unit.
 *
 *    Name: game_state
 *    Type: Athena_GameState *
 *    Free: caller
 *    Note: This is always the first arg (if any args exist at all)
 *
 *    Name: screen_state
 *    Type: int *
 *    Free: caller
 *    Note: This is only used in UI screens, such as the title screen or options screen.
 *              Usually, -1 means "in progress", -2 means "quit", and >= 0 is an enumeration of options.
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
struct Athena_Button *Athena_AppendButtonList(struct Athena_ButtonList **to, struct Athena_ButtonList *button);
struct Athena_Button *Athena_AppendButton(struct Athena_ButtonList **to, struct Athena_Button button);
int Athena_FreeButtonArgList(struct Athena_ButtonArgList *args);
struct Athena_ButtonArgList *Athena_DefaultButtonArgList(struct Athena_GameState *arg);

/* type will NOT be freed (to allow literal constants more easily) */
int Athena_AppendButtonArgList(struct Athena_ButtonArgList *list, void *arg, const char *type);
void *Athena_FindTypeInArgList(struct Athena_ButtonArgList *list, const char *type);
void Athena_CopyButtonArgList(struct Athena_ButtonArgList **to, struct Athena_ButtonArgList *from);

struct Athena_Event;
int Athena_ProcessButtons(struct Athena_GameState *that, struct Athena_ButtonList *buttons, const struct Athena_Event *event, struct Athena_MessageList *messages);
