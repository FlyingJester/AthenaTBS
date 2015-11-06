#pragma once
#include "image.h"
#include "viewport.h"

struct Athena_Player{
    int cash, metal, food;
    char *name;
    struct Athena_Image flag;
    const uint32_t color;
    unsigned human;
};

void Athena_DrawPlayerDataBox(const struct Athena_Player *player, struct Athena_Viewport *to);
