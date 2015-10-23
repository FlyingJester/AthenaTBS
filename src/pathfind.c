#include "pathfind.h"
#include "unit.h"
#include <setjmp.h>
#include <stdlib.h>

/*
static void athena_append_position(struct Athena_PositionList **in, int x, int y){
    if(!in[0]){
        in[0] = malloc(sizeof(struct Athena_PositionList));
        in[0]->x = x;
        in[0]->y = y;
    }
    else{
        athena_append_position(&(in[0]->next), x, y);
    }
}
*/

static void athena_attack_range_positions_iter(jmp_buf buffer, 
    struct Athena_PositionList **in_list, struct Athena_PositionList **out_list,
    const struct Athena_Unit *unit){
    
    
}

struct Athena_PositionList *Athena_AttackRangePositions(struct Athena_ButtonArgList *args){
    jmp_buf buffer;
    struct Athena_PositionList *out_list = NULL,
        * in_list = malloc(sizeof(struct Athena_PositionList));

    const struct Athena_Unit *const unit = Athena_FindTypeInArgList(args, "source_unit");

    in_list->x = unit->x;
    in_list->y = unit->y;
    in_list->next = NULL;

    if(setjmp(buffer)){
        if(!in_list)
            return out_list;
    }

    athena_attack_range_positions_iter(buffer, &in_list, &out_list, unit);

    /* This will basically never be reached. But oh well. */
    return out_list;
}

struct Athena_PositionList *Athena_MovementPositions(struct Athena_ButtonArgList *arg){
    return NULL;
}
