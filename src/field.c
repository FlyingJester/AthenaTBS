#include "field.h"
#include "bufferfile/bufferfile.h"
#include <TurboJSON/value.h>
#include <TurboJSON/object.h>
#include "turbo_json_helpers.h"

unsigned Athena_MovementCost(unsigned x, unsigned y, const struct Athena_Field *field){
    /* Lol. */
    return 1;
}

unsigned short Athena_TileIndexAt(const struct Athena_Field *field, unsigned x, unsigned y){
    return field->field.indices[ x + (y * field->w) ];
}

static int athena_draw_field_iter(const struct Athena_Field *field, struct Athena_Image *onto, unsigned row, int x, int y){
    if(row >= field->h)
        return 0;
    else if(y >= onto->h)
        return 0;
    else{
        if(y + (long)field->tileset->tile_height >=0 ){
            Athena_DrawTileRow(field->tileset, onto, &field->field, row * field->w, field->w, x, y);
        }
        return athena_draw_field_iter(field, onto, row + 1, x, y + field->tileset->tile_height);
    }
}

int Athena_DrawField(const struct Athena_Field *field, struct Athena_Image *onto, int x, int y){
    if(x + (long)field->tileset->tile_width < 0)
        return 0;
    return athena_draw_field_iter(field, onto, 0, x, y);
}

int Athena_LoadFieldFromFile(const char *file, struct Athena_Field *to){
    int size, ret;
    void * const data = BufferFile(file, &size);
    
    if(!data)
        return -1;
    
    ret = Athena_LoadFieldFromMemory(data, size, to);
    
    FreeBufferFile(data, size);
    
    return ret;
}

int Athena_LoadFieldFromMemory(const void *data, unsigned len, struct Athena_Field *to){
    struct Turbo_Value value;
    const char * const source = data;
    Turbo_Value(&value, source, source + len);
    if(value.type==TJ_Object)
        return Athena_LoadFieldFromTurboValue(&value, to);
    return -1;
}

int Athena_LoadFieldFromTurboValue(const struct Turbo_Value *value, struct Athena_Field *to){
    const struct Turbo_Value * const tileset = Turbo_Helper_GetConstObjectElement(value, "tileset"),
        * const field = Turbo_Helper_GetConstObjectElement(value, "tileset");

    if(!tileset || !field)
        return -1;

    if(tileset->type != TJ_String || field->type != TJ_Array)
        return -1;

    return 0;
}
