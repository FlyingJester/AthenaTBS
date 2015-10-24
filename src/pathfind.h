#pragma once
#include "button.h"

struct Athena_PositionList{
    int x, y;
    struct Athena_PositionList *next;
};

void Athena_FoldPositions(const struct Athena_PositionList *pos, void(*callback)(void *arg, int x, int y), void *arg);

struct Athena_PositionList *Athena_AttackRangePositions(struct Athena_ButtonArgList *arg);
struct Athena_PositionList *Athena_MovementPositions(struct Athena_ButtonArgList *arg);
struct Athena_PositionList *Athena_DumbMovementPositions(struct Athena_ButtonArgList *arg);
