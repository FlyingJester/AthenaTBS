#pragma once
#include "resources.h"
#include "image.h"
#include "viewport.h"
#include "tech_tree.h"

struct Athena_UnitList;

struct Athena_Player{
    struct Athena_Resources resources;
    char *name;
    struct Athena_Image flag;
    const uint32_t color;
    unsigned human;
    
    unsigned num_buildable, buildable_capacity;
    struct Athena_Class **buildable;
    
    const char **tech;
    unsigned num_tech, tech_capacity;
};

unsigned Athena_CanBuild(const struct Athena_Player *player, struct Athena_Class *class);

/* Will return at most 16. Returns the number of matches filled in. */
unsigned Athena_GetBuilds(const struct Athena_Player *player, const char *builder, const struct Athena_Class **out);

void Athena_GivePlayerIncome(struct Athena_Player *player, const struct Athena_UnitList *units);
void Athena_PlayerIncome(const struct Athena_Player *player, const struct Athena_UnitList *units, struct Athena_Resources *to);

void Athena_DrawPlayerDataBox(const struct Athena_Player *player, struct Athena_Viewport *to);
unsigned Athena_CanPayResources(const struct Athena_Player *player, const struct Athena_Resources *amount);
unsigned Athena_PayResources(struct Athena_Player *player, const struct Athena_Resources *amount);


unsigned Athena_PlayerMeetsReqsBonus(const struct Athena_Player *player, const struct Athena_BonusList *bonus);
