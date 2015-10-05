#pragma once
#include "spriteset.h"

/* For an equal attack and defense, this is the number of attacks it takes to defeat a unit. */
#define ATTACK_CONSTANT 4.0f

struct Athena_Class {
    float defense, attack;
    unsigned speed;
    unsigned range;
    struct Athena_Spriteset *spriteset;
};

struct Athena_Unit {
    const struct Athena_Class *clazz;
    unsigned owner;
    float health;
    unsigned x, y, movement, attacks;
};

struct Athena_UnitList {
    struct Athena_Unit unit;
    struct Athena_UnitList *next;
};

void Athena_Attack(const struct Athena_Unit *attacker, struct Athena_Unit *other);
struct Athena_Unit *Athena_UnitsCollide(struct Athena_UnitList *list, const unsigned x, const unsigned y);
unsigned Athena_UnitDistance(const struct Athena_Unit *a, const struct Athena_Unit *b);
