#include "tech_tree.h"
#include "window_style.h"
#include <stdlib.h>
#include <string.h>

static void athena_draw_tech_tree_iter(struct Athena_Viewport *to, struct Athena_ClassList *clazzes){
    if(clazzes){
        
        
        
        Athena_ShrinkViewport(to, 0, 24, 0, 0);
        athena_draw_tech_tree_iter(to, clazzes->next);
    }
}

void Athena_DrawTechTree(struct Athena_TechTree *tree, struct Athena_Viewport *to){
    struct Athena_Viewport p = *to;
    Athena_DrawDefaultWindowStyle(to);
    Athena_ShrinkViewport(&p, 4, 4, 4, 4);
    athena_draw_tech_tree_iter(&p, tree->clazzes);
}

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

void Athena_ForEachTechBonus(struct Athena_TechTree *tree, void(*callback)(struct Athena_Bonus *bonus, void *a), void *arg){
    Athena_ForEachBonus(tree->bonuses, callback, arg);
}

void Athena_ForEachBonus(struct Athena_BonusList *list, void(*callback)(struct Athena_Bonus *bonus, void *a), void *arg){
    if(list){
        callback(&list->bonus, arg);
        Athena_ForEachBonus(list->next, callback, arg);
    }
}

void Athena_ForEachTechClass(struct Athena_TechTree *tree, void(*callback)(struct Athena_Class *clazz, void *a), void *arg){
    Athena_ForEachClass(tree->clazzes, callback, arg);
}

void Athena_ForEachClass(struct Athena_ClassList *list, void(*callback)(struct Athena_Class *clazz, void *a), void *arg){
    if(list){
        callback(list->clazz, arg);
        Athena_ForEachClass(list->next, callback, arg);
    }
}

struct athena_bonus_struct_1{
    const char *what; 
    long int a;
};

static void athena_bonus_type_accumulate_callback(struct Athena_Bonus *bonus, void *arg){
    struct athena_bonus_struct_1 *b = arg;
    if(strcmp(bonus->what, b->what)==0)
        b->a += bonus->amount;
}

long int Athena_AccumulateTechBonus(struct Athena_TechTree *tree, const char *bonus_name){
    struct athena_bonus_struct_1 arg = { NULL, 0l };
    arg.what = bonus_name;

    Athena_ForEachTechBonus(tree, athena_bonus_type_accumulate_callback, &arg);
    
    return arg.a;
}
