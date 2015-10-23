#pragma once
#include "button.h"

struct Athena_PositionList{
    int x, y;
    struct Athena_PositionList *next;
};

struct Athena_PositionList *Athena_AttackRangePositions(struct Athena_ButtonArgList *arg);
struct Athena_PositionList *Athena_MovementPositions(struct Athena_ButtonArgList *arg);
