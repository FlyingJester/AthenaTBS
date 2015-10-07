#include "field.h"
#include "tileset.h"
#include "path/path.h"
#include "bufferfile/bufferfile.h"
#include <TurboJSON/value.h>
#include <TurboJSON/object.h>
#include "turbo_json_helpers.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
    int size, ret = -1;
    void * const data = BufferFile(file, &size);
    
    if(data){
        char * const directory = Athena_GetContainingDirectory(file);
        ret = Athena_LoadFieldFromMemory(data, size, to, directory);
        free(directory);
        FreeBufferFile(data, size);
    }
    
    return ret;
}

int Athena_LoadFieldFromMemory(const void *data, unsigned len, struct Athena_Field *to, const char *directory){
    struct Turbo_Value value;
    const char * const source = data;
    Turbo_Value(&value, source, source + len);
    if(value.type==TJ_Object)
        return Athena_LoadFieldFromTurboValue(&value, to, directory);
    return -1;
}

static int athena_load_field_row(const unsigned w, unsigned short *field_i, const struct Turbo_Value *row_array){
    if(w){
        if(row_array->type == TJ_Number)
            field_i[0] = row_array->value.number;
        else{
            fprintf(stderr, "[athena_load_field_row]Value in column -%i is not a number.\n", (w));
            field_i[0] = 0u;
        }
        return athena_load_field_row(w - 1, field_i + 1, row_array + 1);
    }
    else
        return 0;
}

static int athena_load_field_indices_rows(const unsigned w, unsigned remaining_rows, unsigned short *field_i, const struct Turbo_Value *row_array){
    if(remaining_rows==0)
        return 0;
    else{
        if(row_array->type != TJ_Array){
            fprintf(stderr, "[athena_load_field_indices_row]Invalid row -%i is not an array.\n", (int)remaining_rows);
            memset(field_i, 0, sizeof(unsigned short) * w);
        }
        else if(row_array->length < w){
            fprintf(stderr, 
                "[athena_load_field_indices_row]Invalid row length of %i. Expected %i.\n", 
                (int)row_array->length, (int)w);
            
            athena_load_field_row(row_array->length, field_i, row_array->value.array);
            memset(field_i + row_array->length, 0, (w - row_array->length) * sizeof(unsigned short));
            
        }
        else{
            if(row_array->length > w)
                fprintf(stderr, 
                    "[athena_load_field_indices_row]Row has extra elements. Length is %i. Expected %i.\n", 
                    (int)row_array->length, (int)w);
            
            athena_load_field_row(w, field_i, row_array->value.array);
        }

        return athena_load_field_indices_rows(w, remaining_rows-1, field_i + w, row_array + 1);
    }
}

int Athena_LoadFieldFromTurboValue(const struct Turbo_Value *value, struct Athena_Field *to, const char *directory){
    const struct Turbo_Value * const tileset = Turbo_Helper_GetConstObjectElement(value, "tileset"),
        * const field = Turbo_Helper_GetConstObjectElement(value, "field"),
        * const attributes = Turbo_Helper_GetConstObjectElement(value, "attributes");
    int ret = 0;

    if(!tileset || !field || attributes->type!=TJ_Object || field->type!=TJ_Array){
        return -10;
    }
    else{
        const struct Turbo_Value 
            * const width = Turbo_Helper_GetConstObjectElement(attributes, "width"),
            * const height = Turbo_Helper_GetConstObjectElement(attributes, "height");
        if(height->type != TJ_Number || width->type != TJ_Number)
            return -20;
        
        if(field->length < to->h){
            return -21;
        }
        
        to->w = width->value.number;
        to->h = height->value.number;

    }

    to->tileset = calloc(sizeof(struct Athena_Tileset), 1);

    if(tileset->type == TJ_String){
        char *filename = malloc(tileset->length + 1);
        memcpy(filename, tileset->value.string, tileset->length);
        filename[tileset->length] = '\0';

        ret = Athena_LoadTilesetFromFile(filename, to->tileset);

        free(filename);
    }
    else if(tileset->type == TJ_Array){
        ret = Athena_LoadTilesetFromTurboValue(tileset, to->tileset, directory);
    }

    if(ret!=0){
        free(to->tileset);
        return -30;
    }
/*
    struct Athena_TileIndexArray{
        unsigned short *indices;
        unsigned num_indices, indices_capacity;
    };
*/
    to->field.indices = malloc(sizeof(unsigned short) * to->w * to->h);
    ret = athena_load_field_indices_rows(to->w, to->h, to->field.indices, field);

    return ret;
}
