#pragma once
#include "game.h"

enum Athena_ServerMessageTypes { 
    Nothing,
    EndTurn,
    MoveUnit,
    AttackUnit,
    BuildUnit,
    BuildTile,
    NumServerMessageTypes
};

const char *Athena_ServerMessageTypeString(enum Athena_ServerMessageTypes);

/* Returns NumServerMessageTypes on error */
enum Athena_ServerMessageTypes Athena_ServerMessageTypeEnumN(const char *str, unsigned len);
enum Athena_ServerMessageTypes Athena_ServerMessageTypeEnum(const char *str);

void Athena_ServerThreadWrapper(void *that);
int Athena_ServerThread(struct Athena_GameState *that);
