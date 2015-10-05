#pragma once
#include "unit.h"
#include "field.h"

struct Athena_Position{ unsigned x, y; };

struct Athena_PositionList{ 
    struct Athena_Position pos;
    struct Athena_PositionList *next;
};

struct Athena_Course{
    struct Athena_PositionList *positions;
    struct Athena_Unit *from, *to;
};

/* Populate the from and to of course, and positions will be filled out. */
int Athena_PlotCourse(struct Athena_Course *course, struct Athena_Field *field);
struct Athena_Unit *Athena_IsObstructed(const struct Athena_Unit *that, struct Athena_Field *field);
