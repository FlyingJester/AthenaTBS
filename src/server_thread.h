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

/* Semi-private helper to get number values of the "to" and "from" objects' "x" and "y" properties inside obj.
 * Object to gather from is like this:
{
    "to":{"x":10, "y":37},
    "from":{"x":98, "y":32}
}
 * Returns 0 on success.
 */
int Athena_GetJSONToAndFrom(const struct Turbo_Value *obj, int *from_x_out, int *from_y_out, int *to_x_out, int *to_y_out);
