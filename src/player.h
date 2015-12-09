#pragma once
#include "image.h"
#include "viewport.h"

struct Athena_UnitList;

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

void Athena_GivePlayerIncome(struct Athena_Player *player, const struct Athena_UnitList *units);
void Athena_PlayerIncome(const struct Athena_Player *player, const struct Athena_UnitList *units, struct Athena_Resources *to);

void Athena_DrawPlayerDataBox(const struct Athena_Player *player, struct Athena_Viewport *to);
unsigned Athena_CanPayResources(const struct Athena_Player *player, const struct Athena_Resources *amount);
unsigned Athena_PayResources(struct Athena_Player *player, const struct Athena_Resources *amount);

/* Semi-private, since they have very little use outside player.c */
void Athena_AddResources(struct Athena_Resources *to, const struct Athena_Resources *from);
void Athena_SubResources(struct Athena_Resources *to, const struct Athena_Resources *from);
