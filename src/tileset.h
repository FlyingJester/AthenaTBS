#pragma once
#include "tile.h"
#include "image.h"

struct Athena_TileArray{
    struct Athena_Tile *tiles;
    unsigned num_tiles, tiles_capacity;
};

struct Athena_Tileset{
    struct Athena_TileArray tiles;
    struct Athena_ImageArray images;
    unsigned tile_width, tile_height;
};

struct Athena_TileIndexArray{
    unsigned short *indices;
    unsigned num_indices, indices_capacity;
};

void Athena_CreateTileset(struct Athena_Tileset *ts);
void Athena_DestroyTileset(struct Athena_Tileset *ts);

/* Returns the index of the inserted object */
unsigned Athena_AddTile(struct Athena_Tileset *ts, struct Athena_Tile *tile);
unsigned Athena_AddImage(struct Athena_Tileset *ts, struct Athena_Image *image);

void Athena_AddImageCopy(struct Athena_Tileset *ts, const struct Athena_Image *image);
void Athena_DrawTileIndex(const struct Athena_Tileset *ts, struct Athena_Image *onto, unsigned index, int x, int y);
void Athena_DrawImageIndex(const struct Athena_Tileset *ts, struct Athena_Image *onto, unsigned index, int x, int y);
void Athena_DrawTileRow(const struct Athena_Tileset *ts, struct Athena_Image *onto, 
    const struct Athena_TileIndexArray *indices, unsigned offset, unsigned len, int x, int y);

int Athena_LoadTilesetFromFile(const char *file, struct Athena_Tileset *to);

/* Semi-private helper functions */
int Athena_LoadTilesetFromMemory(const void *data, unsigned len, struct Athena_Tileset *to, const char *directory);
void Athena_CreateTileArray(struct Athena_TileArray *ts);
void Athena_DestroyTileArray(struct Athena_TileArray *ts);

struct Turbo_Value;
/* value->type _must_ be Object */
int Athena_LoadTilesetFromTurboValue(const struct Turbo_Value *value, struct Athena_Tileset *to, const char *directory);
int Athena_LoadTileArrayFromTurboValue(const struct Turbo_Value *value, unsigned num_values, struct Athena_Tileset *to, const char *directory);

int Athena_TilesetTest();
