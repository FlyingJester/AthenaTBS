#pragma once
#include "image.h"
#include "viewport.h"
#include "unit.h"
#include "resources.h"

struct Athena_Bonus{
    char what[80];
    int amount;
};

/* Used to indicate a bonus is not amount based. */
#define ATHENA_NON_BONUS (-0xFFFF)

struct Athena_BonusList{
    char title[80];
    struct Athena_Bonus bonus;

    const char **reqs;
    unsigned num_reqs, reqs_capacity;

    struct Athena_Resources cost;

    struct Athena_BonusList *next;
};

/* Representing a linked list, each node being a tech level. */
struct Athena_TechTree{
    struct Athena_ClassList *clazzes;
    struct Athena_BonusList *bonuses;
    struct Athena_TechTree *next;
};

const struct Athena_TechTree *Athena_GetDefaultTechTree();
void Athena_InvalidateDefaultTechTree();

/* Partially for testing purposes, overlay callbacks to display the default tech tree. 
 * They do not require any args to be added to the arglist except the GameState */
unsigned Athena_DefaultTechOverlayEvent(struct Athena_ButtonArgList *, const struct Athena_Event *event, struct Athena_MessageList *);
void Athena_DefaultTechOverlayDraw(const struct Athena_ButtonArgList *, struct Athena_Image *framebuffer);

int Athena_LoadTechTreeFromFile(const char *file, struct Athena_TechTree *to);
int Athena_LoadTechTreeFromMemory(const void *data, unsigned len, struct Athena_TechTree *to);

struct Turbo_Value;
/* value->type _must_ be Object */
int Athena_LoadTechTreeFromTurboValue(const struct Turbo_Value *value, struct Athena_TechTree *to);

void Athena_FreeTechTree(struct Athena_TechTree *tree);

/* Simply draws the entire tech tree. This will not be very useful to an actual player. */
void Athena_DrawTechTree(struct Athena_TechTree *tree, struct Athena_Viewport *to);

void Athena_AppendBonus(const char *what, int amount, struct Athena_BonusList **to);
void Athena_AppendBonusN(const char *what, unsigned len, int amount, struct Athena_BonusList **to);
void Athena_AppendClass(struct Athena_Class *clazz, struct Athena_ClassList **to);

void Athena_AppendTechBonus(const char *what, int amount, struct Athena_TechTree *to);
void Athena_AppendTechClass(struct Athena_Class *clazz, struct Athena_TechTree *to);
void Athena_FreeBonusList(struct Athena_BonusList *bonuses);
void Athena_FreeClassList(struct Athena_ClassList *clazzes);

void Athena_AppendTechLevel(struct Athena_TechTree *tree, struct Athena_TechTree **to);

void Athena_ForEachTechBonus(struct Athena_TechTree *tree, void(*callback)(struct Athena_Bonus *bonus, void *a), void *arg);
void Athena_ForEachBonus(struct Athena_BonusList *list, void(*callback)(struct Athena_Bonus *bonus, void *a), void *arg);
void Athena_ForEachTechClass(struct Athena_TechTree *tree, void(*callback)(struct Athena_Class *clazz, void *a), void *arg);
void Athena_ForEachClass(struct Athena_ClassList *list, void(*callback)(struct Athena_Class *clazz, void *a), void *arg);
long int Athena_AccumulateTechBonus(struct Athena_TechTree *tree, const char *bonus_name);
