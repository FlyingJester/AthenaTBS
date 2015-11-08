#include "unit.h"
#include "player.h"
#include "unit_classes.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

int Athena_DrawUnit(struct Athena_Unit *unit, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y){
    if(!unit)
        return 1;
    else{
        const struct Athena_Animation *sprite = &unit->sprite;
        const int to_x = (unit->x * tile_w) - x, to_y = (unit->y * tile_h) - y - ( unit->sprite.frames->frame.image->h - tile_h );
        uint32_t color = Athena_RGBAToRaw(0xFF>>1, 0xFF>>1, 0xFF>>1, 0xFF);
        
        if(unit->owner){
            color = unit->owner->color;
        }
        
        Athena_FillRect(to, unit->x * tile_w, unit->y * tile_h, tile_w, 2, color);
        Athena_FillRect(to, unit->x * tile_w + 2, ((unit->y + 1) * tile_h) - 2, tile_w - 4, 2, color);
        Athena_FillRect(to, unit->x * tile_w, unit->y * tile_h, 2, tile_h, color);
        Athena_FillRect(to, ((unit->x + 1) * tile_w) - 2, unit->y * tile_h, 2, tile_h, color);

        if(unit->actions>0){
            Athena_DrawAnimation(sprite, to, to_x, to_y);
        }
        else{
            Athena_DrawAnimationBlendMode(sprite, to, to_x, to_y, Athena_RGBARawGrayscale);
        }
        
        return Athena_AnimationTick(&unit->sprite);
    }
}

int Athena_DrawUnitList(struct Athena_UnitList *units, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y){
    if(!units)
        return 0;
    else
        return Athena_DrawUnit(&units->unit, to, tile_w, tile_h, x, y) || Athena_DrawUnitList(units->next, to, tile_w, tile_h, x, y);
}

int Athena_DrawUnitHealthBar(struct Athena_Unit *unit, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y){
    if(fabs(unit->health - 1.0f) > 1.0f/256.0f){
        const int to_x = (unit->x * tile_w) - x, to_y = (unit->y * tile_h) - y - (unit->sprite.frames->frame.image->h - tile_h + 4);
        const int fill_w = (tile_w * (unit->health * 255.0f)) / 255.0f;
        Athena_FillRect(to, to_x - 1, to_y, tile_w + 2, 3, Athena_RGBAToRaw(0x40, 0x40, 0x40, 0xFF));
        Athena_FillRect(to, to_x, to_y + 1, fill_w, 1, Athena_RGBAToRaw(0x20, 0xB0, 0x20, 0xFF));
    }
    return 0;
}

int Athena_DrawUnitListHealthBar(struct Athena_UnitList *units, struct Athena_Image *to, unsigned tile_w, unsigned tile_h, int x, int y){
    if(!units)
        return 0;
    else
        return Athena_DrawUnitHealthBar(&units->unit, to, tile_w, tile_h, x, y) || Athena_DrawUnitListHealthBar(units->next, to, tile_w, tile_h, x, y);
}

struct Athena_Unit *Athena_AppendUnit(struct Athena_UnitList **units){
    if(units[0])
        return Athena_AppendUnit(&(units[0]->next));
    else{
        units[0] = calloc(1, sizeof(struct Athena_UnitList));
        return &units[0]->unit;
    }
}

void Athena_CreateUnit(struct Athena_Unit *to, const struct Athena_Class *clazz, struct Athena_Player *owner, unsigned x, unsigned y){
    to->clazz = clazz;
    to->health = 1.0f;
    to->x = x;
    to->y = y;
    to->owner = owner;
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

struct Athena_Unit *Athena_FindUnitTypeAtN(struct Athena_UnitList *list, const char *name, unsigned name_len, int x, int y){
    if(!list)
        return NULL;
    if(list->unit.x == x && list->unit.y == y && strlen(list->unit.clazz->name) == name_len && memcmp(list->unit.clazz->name, name, name_len) == 0)
        return &(list->unit);
    return Athena_FindUnitTypeAtN(list->next, name, name_len, x, y);
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

void Athena_RenewUnitList(struct Athena_UnitList *unit){
    if(unit){
        Athena_RenewUnit(&unit->unit);
        Athena_RenewUnitList(unit->next);
    }
}

void Athena_RenewUnitListIf(struct Athena_UnitList *unit, int(*check)(void *arg, const struct Athena_Unit *unit), void *arg){
    if(unit){
        struct Athena_Unit *const that = &unit->unit;
        if(check(arg, that))
            Athena_RenewUnit(that);

        Athena_RenewUnitListIf(unit->next, check, arg);
    }
}

int Athena_CheckUnitOwner(const struct Athena_CheckUnitOwnerData *data, const struct Athena_Unit *unit){
    if(unit->owner == data->owner)
        return !data->toggle;
    else
        return data->toggle;
}

int Athena_CheckUnitOwnerCallback(void *arg, const struct Athena_Unit *unit){
    return Athena_CheckUnitOwner(arg, unit);
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
