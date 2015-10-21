#include "unit.h"
#include "unit_classes.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

int Athena_DrawUnit(struct Athena_Unit *unit, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y){
    if(!unit)
        return 1;
    else if(Athena_DrawAnimation(&unit->sprite, to, (unit->x * tile_w) - x, (unit->y * tile_h) - y - ( unit->sprite.frames->frame->h - tile_h ))!=0)
        return 1;
    else
        return Athena_AnimationTick(&unit->sprite);
}

int Athena_DrawUnitList(struct Athena_UnitList *units, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y){
    if(!units)
        return 0;
    else
        return Athena_DrawUnit(&units->unit, to, tile_w, tile_h, x, y) || Athena_DrawUnitList(units->next, to, tile_w, tile_h, x, y);
}

struct Athena_Unit *Athena_AppendUnit(struct Athena_UnitList **units){
    if(units[0])
        return Athena_AppendUnit(&(units[0]->next));
    else{
        units[0] = calloc(1, sizeof(struct Athena_UnitList));
        return &units[0]->unit;
    }
}

void Athena_CreateUnit(struct Athena_Unit *to, const struct Athena_Class *clazz, unsigned owner, unsigned x, unsigned y){
    to->clazz = clazz;
    to->health = 1.0f;
    to->x = x;
    to->y = y;
    Athena_DepleteUnit(to);
    to->sprite.frames = Athena_GetSpritesetDirection(clazz->spriteset, "idle", "south", NULL);
    to->sprite.last_time = 0;
}

void Athena_Attack(const struct Athena_Unit *attacker, struct Athena_Unit *other){
    other->health -= (other->clazz->attack / other->clazz->defense) / ATTACK_CONSTANT;
    if(other->health<0.0f)
        other->health = 0.0f;
}

struct Athena_Unit *Athena_FindUnitAt(struct Athena_UnitList *list, int x, int y){
    if(!list)
        return NULL;
    if(list->unit.x == x && list->unit.y == y)
        return &(list->unit);
    return Athena_FindUnitAt(list->next, x, y);
}

unsigned Athena_UnitDistance(const struct Athena_Unit *a, const struct Athena_Unit *b){
    const int delta_x = a->x - b->x,
        delta_y = a->y - b->y;
    return sqrt(delta_x * delta_x + delta_y * delta_y);
}

void Athena_RenewUnit(struct Athena_Unit *to){
    to->movement = to->clazz->movement;
    to->actions = to->clazz->actions;
}

void Athena_DepleteUnit(struct Athena_Unit *to){
    to->movement = 0;
    to->actions = 0;
}

static const struct Athena_Class *athena_builtin_class(const char *name, unsigned i){
    if(i>=ATHENA_NUM_UNIT_CLASSES)
        return NULL;
    else if(strcmp(name, athena_unit_classes[i].name)==0)
        return athena_unit_classes + i;
    else
        return athena_builtin_class(name, i+1);
}

const struct Athena_Class *Athena_BuiltinClass(const char *name){
    return athena_builtin_class(name, 0);
}
