#include "pathfind.h"
#include "unit.h"
#include "field.h"
#include "game.h"
#include <stdlib.h>
#include <math.h>

static float athena_position_distance(int x1, int y1, int x2, int y2){
    const int dy = y1 - y2, dx = x1 - x2, as = dx * dx, bs = dy * dy;
    return sqrt(as + bs);
}

#define POS_IN_LIST_DEF(X, TYPE)\
struct TYPE *Athena_ ## X ## InList(struct TYPE *list, int x, int y){\
    if(!list || (list->x == x && list->y == y))\
        return list;\
    else\
        return Athena_ ## X ## InList(list->next, x, y);\
}

#define FREE_POS_LIST_DEF(X, TYPE)\
void Athena_Free ## X ## List(struct TYPE *list){\
    if(list){\
        struct TYPE * const next = list->next;\
        free(list);\
        Athena_Free ## X ## List(next);\
    }\
}

POS_IN_LIST_DEF(Position, Athena_PositionList)
FREE_POS_LIST_DEF(Position, Athena_PositionList)

/* Simple distances ********************
 ***************************************
 ***************************************/

void Athena_FoldPositions(const struct Athena_PositionList *pos, void(*callback)(void *arg, int x, int y), void *arg){
    if(pos){
        callback(arg, pos->x, pos->y);
        Athena_FoldPositions(pos->next, callback, arg);
    }
}

static void athena_append_position(struct Athena_PositionList **in, int x, int y){
    if(!in[0]){
        in[0] = malloc(sizeof(struct Athena_PositionList));
        in[0]->x = x;
        in[0]->y = y;
        in[0]->next = NULL;
    }
    else{
        athena_append_position(&(in[0]->next), x, y);
    }
}

static void athena_try_add_attack_position(int x, int y, const struct Athena_Unit *unit, 
    struct Athena_PositionList **in_list, struct Athena_PositionList **out_list, struct Athena_PositionList **dead_list){
    if(Athena_PositionInList(in_list[0], x, y) ||
        Athena_PositionInList(out_list[0], x, y) ||
        Athena_PositionInList(dead_list[0], x, y) ||
        (x==unit->x && y == unit->y))
        return;
    if(athena_position_distance(x, y, unit->x, unit->y) <= unit->clazz->range){
        athena_append_position(out_list, x, y);
        athena_append_position(in_list, x, y);
    }
    else{
        athena_append_position(dead_list, x, y);
    }
}

static void athena_attack_range_positions_iter(const struct Athena_Unit *unit,
    struct Athena_PositionList **in_list, struct Athena_PositionList **out_list, struct Athena_PositionList **dead_list){
    
    struct Athena_PositionList *pos = in_list[0];
    if(!pos)
        return;
    else{
    
        in_list[0] = pos->next;

        athena_try_add_attack_position(pos->x + 1, pos->y, unit, in_list, out_list, dead_list);
        athena_try_add_attack_position(pos->x - 1, pos->y, unit, in_list, out_list, dead_list);
        athena_try_add_attack_position(pos->x, pos->y + 1, unit, in_list, out_list, dead_list);
        athena_try_add_attack_position(pos->x, pos->y - 1, unit, in_list, out_list, dead_list);

        free(pos);
        
        athena_attack_range_positions_iter(unit, in_list, out_list, dead_list);
    }
}

static void athena_attack_range_inner(const struct Athena_Unit *unit,
    struct Athena_PositionList **in_list, struct Athena_PositionList **out_list, struct Athena_PositionList **dead_list){
    
    if(!in_list[0])
        return;
    else{
        athena_attack_range_positions_iter(unit, in_list, out_list, dead_list);
        athena_attack_range_inner(unit, in_list, out_list, dead_list);
    }
}

struct Athena_PositionList *Athena_AttackRangePositions(struct Athena_ButtonArgList *args){
    struct Athena_PositionList *out_list = NULL, *dead_list = NULL,
        * in_list = malloc(sizeof(struct Athena_PositionList));

    const struct Athena_Unit *const unit = Athena_FindTypeInArgList(args, "source_unit");

    if(unit){

        in_list->x = unit->x;
        in_list->y = unit->y;
        in_list->next = NULL;

        athena_attack_range_inner(unit, &in_list, &out_list, &dead_list);
        
    }
    
    Athena_FreePositionList(dead_list);
    
    return out_list;
}

/* Breadth-first Pathfinding ***********
 ***************************************
 ***************************************/

struct Athena_BreadthPositionList{
    int distance;
    int x, y;
    struct Athena_BreadthPositionList *next;
};

POS_IN_LIST_DEF(BreadthPosition, Athena_BreadthPositionList)
FREE_POS_LIST_DEF(BreadthPosition, Athena_BreadthPositionList)

static void athena_breadth_to_position_list(struct Athena_PositionList **out, const struct Athena_BreadthPositionList *in){
    if(in){
        athena_append_position(out, in->x, in->y);
        athena_breadth_to_position_list(out, in->next);
    }
}

static void athena_append_breadth_position(struct Athena_BreadthPositionList **in, int x, int y, int d){
    if(!in[0]){
        in[0] = malloc(sizeof(struct Athena_BreadthPositionList));
        in[0]->x = x;
        in[0]->y = y;
        in[0]->distance = d;
        in[0]->next = NULL;
    }
    else{
        athena_append_breadth_position(&(in[0]->next), x, y, d);
    }
}

static void athena_try_add_breadth_position(int x, int y, int d, const struct Athena_Field *field, 
    struct Athena_BreadthPositionList **in_list, struct Athena_BreadthPositionList **out_list, struct Athena_BreadthPositionList **dead_list){
    
    if(Athena_BreadthPositionInList(*out_list, x, y)){
        return;
    }
    else{
        const struct Athena_Tile *t = Athena_TileInField(field, x, y);
        if(t){
            const int new_distance = (d - t->obstruction_value) - 1;
            if(new_distance>=0){
                struct Athena_BreadthPositionList * const in_in_list = Athena_BreadthPositionInList(*in_list, x, y);
                if(in_in_list && in_in_list->distance < new_distance){
                    in_in_list->distance = new_distance;
                }
                else{
                    athena_append_breadth_position(in_list, x, y, new_distance);
                }
                athena_append_breadth_position(out_list, x, y, new_distance);
            }
        }
    }
}

static void athena_breadth_positions_iter(const struct Athena_Field *field,
    struct Athena_BreadthPositionList **in_list, struct Athena_BreadthPositionList **out_list, struct Athena_BreadthPositionList **dead_list){
    
    struct Athena_BreadthPositionList *pos = in_list[0];
    if(!pos)
        return;
    else{
    
        in_list[0] = pos->next;

        athena_try_add_breadth_position(pos->x + 1, pos->y, pos->distance, field, in_list, out_list, dead_list);
        athena_try_add_breadth_position(pos->x - 1, pos->y, pos->distance, field, in_list, out_list, dead_list);
        athena_try_add_breadth_position(pos->x, pos->y + 1, pos->distance, field, in_list, out_list, dead_list);
        athena_try_add_breadth_position(pos->x, pos->y - 1, pos->distance, field, in_list, out_list, dead_list);

        free(pos);
        
        athena_breadth_positions_iter(field, in_list, out_list, dead_list);
    }
}

static void athena_breadth_inner(const struct Athena_Field *field,
    struct Athena_BreadthPositionList **in_list, struct Athena_BreadthPositionList **out_list, struct Athena_BreadthPositionList **dead_list){
    
    if(!in_list[0])
        return;
    else{
        athena_breadth_positions_iter(field, in_list, out_list, dead_list);
        athena_breadth_inner(field, in_list, out_list, dead_list);
    }
}

struct Athena_PositionList *Athena_MovementPositions(struct Athena_ButtonArgList *args){
    struct Athena_BreadthPositionList *out_list = NULL, *dead_list = NULL,
        * in_list = malloc(sizeof(struct Athena_BreadthPositionList));

    const struct Athena_Unit *const unit = Athena_FindTypeInArgList(args, "source_unit");
    const struct Athena_GameState * const state = Athena_FindTypeInArgList(args, "game_state");

    if(unit){

        in_list->x = unit->x;
        in_list->y = unit->y;
        in_list->distance = unit->clazz->movement;
        in_list->next = NULL;

        athena_breadth_inner(state->field, &in_list, &out_list, &dead_list);
    }
    
    {
        struct Athena_PositionList *pos_list = NULL;
        athena_breadth_to_position_list(&pos_list, out_list);
        
        Athena_FreeBreadthPositionList(out_list);
        Athena_FreeBreadthPositionList(dead_list);
        
        return pos_list;
    }
}
