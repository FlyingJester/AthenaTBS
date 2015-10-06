#pragma once
#include "field.h"
#include "monitor/monitor.h"
#include "window/window.h"
#include <TurboJSON/value.h>

struct Athena_MessageList{
    struct Turbo_Value value;
    struct Athena_MessageList *next;
};

/* Event queue and locking monitor to get events on. `status' will be made -1 on exit or error. */
struct Athena_EventData{
    struct Athena_Monitor *monitor;
    struct Athena_MessageList *msg;
    int status;
};

struct Athena_GameState{
    struct Athena_Field *field;
    struct Athena_EventData event;
    struct Athena_Player *players;
    unsigned whose_turn, num_players;
};

typedef unsigned (*athena_field_condition)(const struct Athena_Field *field, unsigned num_players);
unsigned Athena_ConquestCondition(const struct Athena_Field *field, unsigned num_players);

int Athena_Game(struct Athena_Field *field, unsigned num_players, athena_field_condition win_condition);
void Athena_AppendMessageList(struct Athena_MessageList **to, struct Athena_MessageList *next);
