#pragma once
#include "unit.h"

struct Athena_Bonus{
    const char *what;
    int amount;
};

struct Athena_BonusList{
    struct Athena_Bonus bonus;
    struct Athena_BonusList *next;
};

/* Representing a linked list, each node being a tech level. */
struct Athena_TechTree{
    struct Athena_ClassList *clazzes;
    struct Athena_BonusList *bonuses;
    struct Athena_TechTree *next;
};

void Athena_AppendBonus(const char *what, int amount, struct Athena_BonusList **to);
void Athena_AppendClass(struct Athena_Class *clazz, struct Athena_ClassList **to);

void Athena_AppendTechBonus(const char *what, int amount, struct Athena_TechTree *to);
void Athena_AppendTechClass(struct Athena_Class *clazz, struct Athena_TechTree *to);

void Athena_AppendTechLevel(struct Athena_TechTree *tree, struct Athena_TechTree **to);
