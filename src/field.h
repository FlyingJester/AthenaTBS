#pragma once
#include "unit.h"
#include "tile.h"
#include "tileset.h"
#include "image.h"

struct Athena_Field {
    struct Athena_UnitList *units;

    unsigned short w, h;
    struct Athena_TileIndexArray field;
    
    struct Athena_Tileset *tileset;
};

unsigned Athena_MovementCost(unsigned x, unsigned y, const struct Athena_Field *field);
int Athena_DrawField(const struct Athena_Field *field, struct Athena_Image *to, int x, int y);
int Athena_LoadFieldFromFile(const char *file, struct Athena_Field *to);

int Athena_FieldTileXYToPixelXY(const struct Athena_Field *field, int x, int y, int *x_to, int *y_to);
int Athena_FieldPixelXYToTileXY(const struct Athena_Field *field, int x, int y, int *x_to, int *y_to);

const struct Athena_Tile *Athena_TileInField(const struct Athena_Field *field, unsigned x, unsigned y);

/* Semi-private helper functions */
unsigned short Athena_TileIndexAt(const struct Athena_Field *field, unsigned x, unsigned y);
int Athena_LoadFieldFromMemory(const void *data, unsigned len, struct Athena_Field *to, const char *directory);

struct Turbo_Value;
/* value->type _must_ be Object */
int Athena_LoadFieldFromTurboValue(const struct Turbo_Value *value, struct Athena_Field *to, const char *directory);

