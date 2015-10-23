#pragma once
#include "field.h"
#include "image.h"
#include "button.h"
#include "menu.h"
#include "pathfind.h"
#include "audio/audio.h"
#include "monitor/monitor.h"
#include "window/window.h"
#include <TurboJSON/value.h>

struct Athena_MessageList{
    /* This is the owning pointer to this text */
    char *msg_text;
    struct Turbo_Value value;
    struct Athena_MessageList *next;
};

/* Event queue and locking monitor to get events on. `status' will be made -1 on exit or error. */
struct Athena_EventData{
    struct Athena_MessageList *msg;
};

struct Athena_SelectingPosition{
    struct Athena_Unit *unit;

    /* In tile coordinates */
    int x, y;
};

/* UI and graphics data. This must only ever be touched by the UI thread. */
struct Athena_UI{
    int camera_x, camera_y;
    struct Athena_Image framebuffer;
    struct Athena_Window *window;
    struct Athena_ButtonList *buttons;
    struct Athena_Sound *click_sound;

    /*
     * When callback is not null, we are selecting a position on the map.
     * The result is stored as Athena_SelectingPosition appended to the end
     * of the arglist that was given.
     * The selection_arg is always freed _after_ the callback is used, and
     * the callback is nulled. We do not keep trying to get a selection if
     * there was a failure.
     */
    struct Athena_ButtonArgList *selection_arg;
    void (*selection_callback)(struct Athena_ButtonArgList *, struct Athena_MessageList *);
    
    /* This callback is nulled at the same time selection_callback is.
     * it populates a list of spaces that are valid targets.
     * The arg is freed at the same time that selection_arg is.
     */
    struct Athena_ButtonArgList *positions_arg;
    struct Athena_PositionList *(*positions_callback)(struct Athena_ButtonArgList *);

    /* Yes, we have only one menu open at a time. */
    struct Athena_Menu *menu;
};

struct Athena_GameState{
    struct Athena_Monitor *monitor;
    struct Athena_Field *field;
    struct Athena_EventData event;
    struct Athena_Player *players;
    struct Athena_UI ui;
    unsigned whose_turn, num_players;
    int status;
};

typedef unsigned (*athena_field_condition)(const struct Athena_Field *field, unsigned num_players);
unsigned Athena_ConquestCondition(const struct Athena_Field *field, unsigned num_players);

int Athena_Game(struct Athena_Field *field, unsigned num_players, struct Athena_Player *players, struct Athena_Window *window, athena_field_condition win_condition);
void Athena_AppendMessageList(struct Athena_MessageList **to, struct Athena_MessageList *next);

/* Message Creation facilities */
char *Athena_CreateEndTurnMessage(int *size);
