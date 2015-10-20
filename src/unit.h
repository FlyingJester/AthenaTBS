#pragma once
#include "spriteset.h"

/* For an equal attack and defense, this is the number of attacks it takes to defeat a unit. */
#define ATTACK_CONSTANT 4.0f

struct Athena_Unit;
struct Athena_Field;

typedef int (*Athena_Ability)(struct Athena_Unit *that, struct Athena_Field *field);

struct Athena_Class {
    float defense, attack;
    unsigned movement, actions;
    unsigned range;
    unsigned is_building, can_build;
    const char *name;
    struct Athena_Spriteset *spriteset;
};

struct Athena_Unit {
    const struct Athena_Class *clazz;
    unsigned owner;
    float health;
    unsigned x, y, movement, actions;
    struct Athena_Animation sprite;
};

struct Athena_UnitList {
    struct Athena_Unit unit;
    struct Athena_UnitList *next;
};

int Athena_DrawUnit(struct Athena_Unit *unit, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y);
int Athena_DrawUnitList(struct Athena_UnitList *units, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y);
#define Athena_DrawUnits Athena_DrawUnitList

struct Athena_Unit *Athena_AppendUnit(struct Athena_UnitList **units);
void Athena_CreateUnit(struct Athena_Unit *to, const struct Athena_Class *clazz, unsigned owner, unsigned x, unsigned y);

#define Athena_SpawnUnit(A_LIST_, A_CLAZZ_, A_OWNER_, A_X_, A_Y_)\
    Athena_CreateUnit(Athena_AppendUnit(A_LIST_), A_CLAZZ_, A_OWNER_, A_X_, A_Y_)

struct Athena_Unit *Athena_FindUnitAt(struct Athena_UnitList *list, int x, int y);

void Athena_Attack(const struct Athena_Unit *attacker, struct Athena_Unit *other);
unsigned Athena_UnitDistance(const struct Athena_Unit *a, const struct Athena_Unit *b);
void Athena_RenewUnit(struct Athena_Unit *unit);
void Athena_DepleteUnit(struct Athena_Unit *unit);

const struct Athena_Class *Athena_BuiltinClass(const char *name);
