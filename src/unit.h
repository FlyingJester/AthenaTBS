#pragma once
#include "spriteset.h"
#include "menu.h"

struct Athena_Player;

/* For an equal attack and defense, this is the number of attacks it takes to defeat a unit. */
#define ATTACK_CONSTANT 4.0f

struct Athena_Unit;
struct Athena_Field;

typedef int (*Athena_Ability)(struct Athena_Unit *that, struct Athena_Field *field);

struct Athena_Class {
    float defense, attack;
    unsigned movement, actions;
    unsigned range;
    unsigned is_building, can_build, is_path;
    const char *name;
    struct Athena_Spriteset *spriteset;
};

struct Athena_Unit {
    const struct Athena_Class *clazz;
    struct Athena_Player *owner;
    float health;
    unsigned x, y, movement, actions;
    struct Athena_Animation sprite;
};

struct Athena_UnitList {
    struct Athena_Unit unit;
    struct Athena_UnitList *next;
};

struct Athena_ClassList{
    struct Athena_Class *clazz;
    struct Athena_ClassList *next;
};

int Athena_DrawUnit(struct Athena_Unit *unit, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y);
int Athena_DrawUnitList(struct Athena_UnitList *units, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y);
#define Athena_DrawUnits Athena_DrawUnitList

/* This is used to draw outlines of units. It's primarily intended to be used to indicate whose unit is which. */
int Athena_DrawUnitHalo(struct Athena_Unit *unit, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y, unsigned radius, uint32_t color);
int Athena_DrawUnitListHalo(struct Athena_UnitList *units, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y);
#define Athena_DrawUnitsHalos Athena_DrawUnitListHalo

int Athena_DrawUnitHealthBar(struct Athena_Unit *unit, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y);
int Athena_DrawUnitListHealthBar(struct Athena_UnitList *units, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y);
#define Athena_DrawUnitsHealthBars Athena_DrawUnitListHealthBar

struct Athena_Unit *Athena_AppendUnit(struct Athena_UnitList **units);
void Athena_CreateUnit(struct Athena_Unit *to, const struct Athena_Class *clazz, struct Athena_Player *owner, unsigned x, unsigned y);

#define Athena_SpawnUnit(A_LIST_, A_CLAZZ_, A_OWNER_, A_X_, A_Y_)\
    Athena_CreateUnit(Athena_AppendUnit(A_LIST_), A_CLAZZ_, A_OWNER_, A_X_, A_Y_)

struct Athena_Unit *Athena_FindUnitAt(struct Athena_UnitList *list, int x, int y);

/* If inverse is 0, then we return the first unit at x, y, for which pred(unit)>0. 
 * Otherwise, we return the first unit at x, y, for which pred(unit)<=0.
 */
struct Athena_Unit *Athena_FindUnitAtWithPredicate(struct Athena_UnitList *list, int x, int y, 
    unsigned(*pred)(const struct Athena_Unit *unit, void *arg), void *arg, unsigned inverse);

/* Some predicates for Athena_FindUnitAtWithPredicate */
unsigned Athena_UnitIsOwnedBy(const struct Athena_Unit *unit, void *arg); /* arg is a struct Athena_Player */
unsigned Athena_UnitIsPassable(const struct Athena_Unit *unit, void *arg); /* arg is a struct Athena_Player */
unsigned Athena_UnitIsBuilding(const struct Athena_Unit *unit, void *arg); /* arg is not used */

struct Athena_UnitList *Athena_FindUnitListAt(struct Athena_UnitList *list, int x, int y);
struct Athena_Unit *Athena_FindUnitTypeAtN(struct Athena_UnitList *list, const char *name, unsigned name_len, int x, int y);
#define Athena_FindUnitTypeAt(LIST_, NAME_, X_, Y_) Athena_FindUnitTypeAtN(LIST_, NAME_, NAME_?strlen(NAME_):0, X_, Y_)
struct Athena_Unit *Athena_FindAttackableUnitAt(struct Athena_UnitList *list, int x, int y);

void Athena_Attack(const struct Athena_Unit *attacker, struct Athena_Unit *other);
unsigned Athena_UnitDistance(const struct Athena_Unit *a, const struct Athena_Unit *b);

void Athena_RenewUnit(struct Athena_Unit *unit);
void Athena_RenewUnitList(struct Athena_UnitList *unit);
void Athena_RenewUnitListIf(struct Athena_UnitList *unit, int(*check)(void *arg, const struct Athena_Unit *unit), void *arg);

void Athena_DepleteUnit(struct Athena_Unit *unit);

struct Athena_CheckUnitOwnerData{
    struct Athena_Player *owner;
    unsigned toggle; /* If this is 1, we return true for units NOT owned by 'owner'. */
};

int Athena_CheckUnitOwner(const struct Athena_CheckUnitOwnerData *data, const struct Athena_Unit *unit);
/* Wraps Athena_CheckUnitOwner */
int Athena_CheckUnitOwnerCallback(void *arg, const struct Athena_Unit *unit);

const struct Athena_Class *Athena_BuiltinClass(const char *name);
