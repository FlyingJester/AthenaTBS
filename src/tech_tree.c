#include "tech_tree.h"
#include <stdlib.h>

void Athena_AppendBonus(const char *what, int amount, struct Athena_BonusList **to){
    if(to[0]!=NULL)
        Athena_AppendBonus(what, amount, &to[0]->next);
    else{
        struct Athena_BonusList *const l = to[0] = malloc(sizeof(struct Athena_BonusList));
        l->bonus.what = what;
        l->bonus.amount = amount;
        l->next = NULL;
    }
}

void Athena_AppendClass(struct Athena_Class *clazz, struct Athena_ClassList **to){
    if(to[0]!=NULL)
        Athena_AppendClass(clazz, &to[0]->next);
    else{
        struct Athena_ClassList *const l = to[0] = malloc(sizeof(struct Athena_ClassList));
        l->clazz = clazz;
        l->next = NULL;
    }
}

void Athena_AppendTechBonus(const char *what, int amount, struct Athena_TechTree *to){
    Athena_AppendBonus(what, amount, &to->bonuses);
}

void Athena_AppendTechClass(struct Athena_Class *clazz, struct Athena_TechTree *to){
    Athena_AppendClass(clazz, &to->clazzes);
}

void Athena_AppendTechLevel(struct Athena_TechTree *tree, struct Athena_TechTree **to){
    if(to[0]==NULL)
        Athena_AppendTechLevel(tree, &to[0]->next);
    else{
        to[0] = tree;
    }
}
