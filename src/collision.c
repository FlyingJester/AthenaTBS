#include "collision.h"
#include "container.h"
#include <stdlib.h>

static struct Athena_Unit *athena_is_obstructed_iter(
    const struct Athena_Unit *that, const struct Athena_Field *field, struct Athena_UnitList *units){

    if(!units)
        return NULL;
    else if(that != &units->unit && that->x == units->unit.x && that->y == units->unit.y){
        return &units->unit;
    }
    else{
        return athena_is_obstructed_iter(that, field, units->next);
    }
}

struct Athena_Unit *Athena_IsObstructed(const struct Athena_Unit *that, struct Athena_Field *field){
    return athena_is_obstructed_iter(that, field, field->units);
}

struct Athena_Vertex{ unsigned x, y, d; };

struct athena_square_array{
    struct Athena_Vertex *data;
    unsigned size, capacity;
};

int Athena_PlotCourse(struct Athena_Course *course, struct Athena_Field *field){
/*    int Athena_CreateQueue(&queue, unsigned element_size, unsigned reserve);*/
    return 0;
}
