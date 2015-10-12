#include "unit.h"
#include <stdlib.h>
#include <math.h>

void Athena_Attack(const struct Athena_Unit *attacker, struct Athena_Unit *other){
    other->health -= (other->clazz->attack / other->clazz->defense) / ATTACK_CONSTANT;
    if(other->health<0.0f)
        other->health = 0.0f;
}

struct Athena_Unit *Athena_UnitsCollide(struct Athena_UnitList *list, const unsigned x, const unsigned y){
    if(!list)
        return NULL;
    if(list->unit.x == x && list->unit.y == y)
        return &(list->unit);
    return Athena_UnitsCollide(list->next, x, y);
}

unsigned Athena_UnitDistance(const struct Athena_Unit *a, const struct Athena_Unit *b){
    const int delta_x = a->x - b->x,
        delta_y = a->y - b->y;
    return sqrt(delta_x * delta_x + delta_y * delta_y);
}

void Athena_RenewUnit(struct Athena_Unit *unit){
    unit->movement = unit->clazz->movement;
    unit->attacks = unit->clazz->attacks;
}

void Athena_LoadSpriteset(struct Athena_Class *class){

}
