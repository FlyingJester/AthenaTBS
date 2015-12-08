#pragma once
#include "image.h"
#include "viewport.h"

struct Athena_Resources{
    int cash, metal, food;
};

struct Athena_Player{
    struct Athena_Resources resources;
    char *name;
    struct Athena_Image flag;
    const uint32_t color;
    unsigned human;
};

void Athena_DrawPlayerDataBox(const struct Athena_Player *player, struct Athena_Viewport *to);
unsigned Athena_CanPayResources(const struct Athena_Resources *payer, const struct Athena_Resources *amount);
unsigned Athena_PayResources(struct Athena_Resources *payer, const struct Athena_Resources *amount);
