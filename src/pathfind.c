#include "pathfind.h"
#include "unit.h"
#include <stdlib.h>
#include <math.h>

static float athena_position_distance(int x1, int y1, int x2, int y2){
    const int dy = y1 - y2, dx = x1 - x2, as = dx * dx, bs = dy * dy;
    return sqrt(as + bs);
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

static struct Athena_PositionList *athena_position_in_list(struct Athena_PositionList *list, int x, int y){
    if(!list || (list->x == x && list->y == y))
        return list;
    else
        return athena_position_in_list(list->next, x, y);
}

static void athena_try_add_attack_position(int x, int y, const struct Athena_Unit *unit, 
    struct Athena_PositionList **in_list, struct Athena_PositionList **out_list, struct Athena_PositionList **dead_list){
    if(athena_position_in_list(in_list[0], x, y) || athena_position_in_list(out_list[0], x, y) || athena_position_in_list(dead_list[0], x, y))
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

    in_list->x = unit->x;
    in_list->y = unit->y;
    in_list->next = NULL;

    athena_attack_range_inner(unit, &in_list, &out_list, &dead_list);

    /* This will basically never be reached. But oh well. */
    return out_list;
}

struct Athena_PositionList *Athena_MovementPositions(struct Athena_ButtonArgList *arg){
    return NULL;
}

