#pragma once
#include "field.h"
#include "image.h"
#include "button.h"
#include "monitor/monitor.h"
#include "window/window.h"
#include <TurboJSON/value.h>

struct Athena_MessageList{
    struct Turbo_Value value;
    struct Athena_MessageList *next;
};

/* Event queue and locking monitor to get events on. `status' will be made -1 on exit or error. */
struct Athena_EventData{
    struct Athena_MessageList *msg;
};

/* UI and graphics data. This must only ever be touched by the UI thread. */
struct Athena_UI{
    int camera_x, camera_y;
    struct Athena_Image framebuffer;
    struct Athena_Window *window;
    struct Athena_ButtonList *buttons;
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
