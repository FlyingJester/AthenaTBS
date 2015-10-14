#include "tileset.h"
#include "container.h"
#include "image.h"
#include "test.h"
#include "turbo_json_helpers.h"
#include "bufferfile/bufferfile.h"
#include "path/path.h"
#include <TurboJSON/parse.h>
#include <TurboJSON/object.h>
#include <stdlib.h>
#include <string.h>

#ifndef ATHENA_MIN
#define ATHENA_MIN(A, B) (((A)<(B))?(A):(B))
#endif

void Athena_CreateTileset(struct Athena_Tileset *ts){
    Athena_CreateTileArray(&ts->tiles);
}

void Athena_DestroyTileset(struct Athena_Tileset *ts){
    Athena_DestroyTileArray(&ts->tiles);
}

void Athena_CreateTileArray(struct Athena_TileArray *ta){
    ta->tiles = malloc(sizeof(struct Athena_Tile) * 8);
    ta->num_tiles = 0;
    ta->tiles_capacity = 8;
}

void Athena_DestroyTileArray(struct Athena_TileArray *ta){
    free(ta->tiles);
    ta->num_tiles = 0;
    ta->tiles_capacity = 0;
}

unsigned Athena_AddTile(struct Athena_Tileset *ts, struct Athena_Tile *tile){
    ts->tiles.tiles = Athena_AssureCapacity(ts->tiles.tiles, sizeof(struct Athena_Tile), ts->tiles.num_tiles+1, &(ts->tiles.tiles_capacity));
    ts->tiles.tiles[ts->tiles.num_tiles++] = *tile;
    return ts->tiles.num_tiles-1;
}

unsigned Athena_AddImageTileset(struct Athena_Tileset *ts, struct Athena_Image *image){
    ts->images.images = Athena_AssureCapacity(ts->images.images, sizeof(struct Athena_Image), ts->images.num_images+1, &(ts->images.images_capacity));
    ts->images.images[ts->images.num_images++] = *image;
    return ts->images.num_images-1;
}

void Athena_DrawTileIndex(const struct Athena_Tileset *ts, struct Athena_Image *onto, unsigned index, int x, int y){
    if(index >= ts->tiles.num_tiles)
        index = ts->tiles.num_tiles - 1;

    Athena_DrawImageIndex(ts, onto, ts->tiles.tiles[index].image_index, x, y);
}

void Athena_DrawImageIndex(const struct Athena_Tileset *ts, struct Athena_Image *onto, unsigned index, int x, int y){
    if(index >= ts->images.num_images)
        index = ts->images.num_images - 1;

    Athena_Blit(&ts->images.images[index], onto, x, y);

}

void athena_draw_tile_row_iter(const struct Athena_Tileset *ts, struct Athena_Image *onto, const struct Athena_TileIndexArray *indices,
    unsigned offset, unsigned len, int x, int y){
    if(x < onto->w && offset<len){
        Athena_DrawTileIndex(ts, onto, indices->indices[offset], x, y);
        athena_draw_tile_row_iter(ts, onto, indices, offset+1, len, x+ts->tile_width, y);
    }
}

void Athena_DrawTileRow(const struct Athena_Tileset *ts, struct Athena_Image *onto, const struct Athena_TileIndexArray *indices,
    unsigned offset, unsigned len, int x, int y){

    if(x + (long)ts->tile_width < 0){
        Athena_DrawTileRow(ts, onto, indices, offset, len, x + ts->tile_width, y);
    }
    else {
        athena_draw_tile_row_iter(ts, onto, indices, offset, ATHENA_MIN(indices->num_indices, offset + len), x, y);
    }
}

int Athena_LoadTilesetFromFile(const char *file, struct Athena_Tileset *to){
    int size;
    void * data = BufferFile(file, &size);
    char *containing_directory = Athena_GetContainingDirectory(file);

    if(!data && size)
        return 1;

    Athena_LoadTilesetFromMemory(data, size, to, containing_directory);

    FreeBufferFile(data, size);
    return 0;
}

int Athena_LoadTilesetFromMemory(const void *z_data, unsigned len, struct Athena_Tileset *to, const char *directory){
    const char * const data = z_data;
    struct Turbo_Value value;
    Turbo_Value(&value, data, data+len);
    
    {
        const int err = Athena_LoadTilesetFromTurboValue(&value, to, directory);
        Turbo_FreeParse(&value);
        return err;
    }
}

int Athena_LoadTilesetFromTurboValue(const struct Turbo_Value *value, struct Athena_Tileset *to, const char *directory){
    memset(to, 0, sizeof(struct Athena_Tileset));

    if(value->type != TJ_Array)
        return 11;
    else{
        const struct Turbo_Value
            * const attribtues = Turbo_Helper_GetConstObjectElement(value->value.array, "attributes");
        if(attribtues){
            const struct Turbo_Value 
                * const width = Turbo_Helper_GetConstObjectElement(attribtues, "width"),
                * const height = Turbo_Helper_GetConstObjectElement(attribtues, "height");
            if(width && width->type == TJ_Number)
                to->tile_width = width->value.number;

            if(height && height->type == TJ_Number)
                to->tile_height = height->value.number;
            
            puts("[Athena_LoadTilesetFromTurboValue]Loaded attributes");
            return Athena_LoadTileArrayFromTurboValue(value->value.array + 1, value->length - 1, to, directory);
        }
        else
            return Athena_LoadTileArrayFromTurboValue(value->value.array, value->length, to, directory);
    }
}

int Athena_LoadTileArrayFromTurboValue(const struct Turbo_Value *value, unsigned num_values, struct Athena_Tileset *to, const char *directory){
    if(num_values==0)
        return 0;
    else if(value->type!=TJ_Object)
        return 21;
    else{
        const struct Turbo_Value
            * const file = Turbo_Helper_GetConstObjectElement(value, "file"),
            * const obstruction_value = 
                Turbo_Helper_GetConstObjectElement(value, "obstruction_value");
        
        if(file->type!=TJ_String)
            return 23;
        else if(obstruction_value->type!=TJ_Number)
            return 24;
        
        else{
            const unsigned dir_length = strlen(directory);
            char * const filename = malloc(file->length + dir_length + 2);
            struct Athena_Image image;

            memcpy(filename, directory, dir_length);
            memcpy(filename + dir_length + 1, file->value.string, file->length);
            filename[dir_length] = '/';
            filename[dir_length + 1 + file->length] = '\0';

            {
                const int err = Athena_LoadAuto(&image, filename);
                free(filename);
                if(err!=0){
                    return 27;
                }
                else{
                    struct Athena_Tile tile;
                    
                    if(!to->tile_width)
                        to->tile_width = image.w;
                    if(!to->tile_height)
                        to->tile_height = image.h;

                    tile.image_index = Athena_AddImageTileset(to, &image);
                    tile.obstruction_value = obstruction_value->value.number;

                    Athena_AddTile(to, &tile);
                    
                    return Athena_LoadTileArrayFromTurboValue(value + 1, num_values - 1, to, directory);
                }
            }
        }
    }
}

/* ========================================================================== */
/* Tileset Tests */
/* ========================================================================== */

int Athena_Test_TilesetAddSingleImage(){

    struct Athena_Tileset ts;
    struct Athena_Image image = { (void *)(0xBEEFDEAD), 16, 24 };
    memset(&ts, 0, sizeof(struct Athena_Tileset));

    Athena_AddImageTileset(&ts, &image);
    
    return ts.images.images[0].pixels == (void *)(0xBEEFDEAD) &&
        ts.images.num_images==1;
}

int Athena_Test_TilesetAddMultipleImages(){

    struct Athena_Tileset ts;
    struct Athena_Image images[2] = {{ (void *)(0xBEEFDEAD), 16, 24 }, {(void *)(0xDEED5555), 16, 16 }};
    memset(&ts, 0, sizeof(struct Athena_Tileset));

    Athena_AddImageTileset(&ts, images);
    Athena_AddImageTileset(&ts, images+1);
    
    return ts.images.images[0].pixels == (void *)(0xBEEFDEAD) &&
        ts.images.images[1].pixels == (void *)(0xDEED5555) &&
        ts.images.num_images==2;
}

int Athena_Test_TilesetAddSingleTile(){

    struct Athena_Tileset ts;
    struct Athena_Tile tile = { 97.0f, 23 };
    memset(&ts, 0, sizeof(struct Athena_Tileset));

    Athena_AddTile(&ts, &tile);
    
    return ts.tiles.tiles[0].image_index == 23 &&
        ts.tiles.tiles[0].obstruction_value == 97.0f &&
        ts.tiles.num_tiles==1;
}

int Athena_Test_TilesetAddMultipleTiles(){

    struct Athena_Tileset ts;
    struct Athena_Tile tiles[2] = {{ 97.0f, 0 }, { 2.03f, 1 }};
    memset(&ts, 0, sizeof(struct Athena_Tileset));

    Athena_AddTile(&ts, tiles);
    Athena_AddTile(&ts, tiles+1);
    
    return ts.tiles.tiles[0].image_index == 0 &&
        ts.tiles.tiles[0].obstruction_value == 97.0f &&
        ts.tiles.tiles[1].image_index == 1 &&
        ts.tiles.tiles[1].obstruction_value == 2.03f &&
        ts.tiles.num_tiles==2;
}

static const char test_tileset1[] = "[\
    {\"attributes\":{ \"width\":24, \"height\":24 }},\
    {\"file\":\"grass1.png\", \"obstruction_value\":1.0 },\
    {\"file\":\"water1.png\", \"obstruction_value\":0.0 }\
]";

int Athena_Test_TilesetFromMemory(){
    struct Athena_Tileset ts;
    int err = Athena_LoadTilesetFromMemory(
        test_tileset1, sizeof test_tileset1, &ts, "res/tests/test_tileset1/");

    if(err==0)
        return 1;
    
    printf("\t[Athena_Test_TilesetFromMemory]: Error %i\n", err);

    return 0;
}

int Athena_Test_TilesetFromFile(){
    struct Athena_Tileset ts;
    int err = Athena_LoadTilesetFromFile("res/tests/test_tileset1/ts.json", &ts);

    if(err==0)
        return 1;
    
    printf("\t[Athena_Test_TilesetFromFile]: Error %i\n", err);

    return 0;
    
    
}

static struct Athena_Test athena_tests[] = {
    ATHENA_TEST(Athena_Test_TilesetAddSingleImage),
    ATHENA_TEST(Athena_Test_TilesetAddMultipleImages),
    ATHENA_TEST(Athena_Test_TilesetAddSingleTile),
    ATHENA_TEST(Athena_Test_TilesetAddMultipleTiles),
    ATHENA_TEST(Athena_Test_TilesetFromMemory),
    ATHENA_TEST(Athena_Test_TilesetFromFile)
};

ATHENA_TEST_FUNCTION(Athena_TilesetTest, athena_tests)
