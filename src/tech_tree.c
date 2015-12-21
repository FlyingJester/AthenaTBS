#include "tech_tree.h"
#include "game.h"
#include "window_style.h"
#include "viewport.h"
#include "font.h"
#include "container.h"
#include <TurboJSON/parse.h>
#include <TurboJSON/object.h>
#include "turbo_json_helpers.h"
#include "bufferfile/bufferfile.h"
#include "window/window.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct Athena_TechTree *system_tech_treep = NULL, system_tech_tree;
const struct Athena_TechTree *Athena_GetDefaultTechTree(){
    if(!system_tech_treep){
        Athena_LoadTechTreeFromFile("res/tech/default.json", system_tech_treep = &system_tech_tree);
    }
    return system_tech_treep;
}

void Athena_InvalidateDefaultTechTree(){
    
}

void Athena_FreeBonusList(struct Athena_BonusList *bonuses){
    if(bonuses){
        struct Athena_BonusList *const next = bonuses->next;
        
        Athena_FreeContainer((void **)(bonuses->reqs), bonuses->num_reqs);
        free((void *)bonuses->reqs);
        free((void *)bonuses);

        Athena_FreeBonusList(next);
    }
}

void Athena_FreeClassList(struct Athena_ClassList *clazzes){
    if(clazzes){
        struct Athena_ClassList *const next = clazzes->next;
        free(clazzes);
        Athena_FreeClassList(next);
    }
}

static void athena_draw_tech_tree_iter(struct Athena_Viewport *to, struct Athena_BonusList *bonuses);

#define MARGINS 8
#define DIMENSIONS 20

static void athena_tech_overlay_close_xy(const struct Athena_Image *im, unsigned *x, unsigned *y){
    x[0] = im->w - ((MARGINS) + DIMENSIONS + 4);
    y[0] = MARGINS + 4;
}

unsigned Athena_DefaultTechOverlayEvent(struct Athena_ButtonArgList *arg, const struct Athena_Event *event, struct Athena_MessageList *msg){
    if(!arg)
        return 0;
    else{
        struct Athena_GameState *const state = (struct Athena_GameState *)(arg->arg);
        unsigned close_x, close_y;
        athena_tech_overlay_close_xy(&state->ui.framebuffer, &close_x, &close_y);
        if(event->type==athena_click_event && (event->which == athena_left_mouse_button || event->which == athena_unknown_mouse_button) && 
            event->x >= close_x && event->y >= close_y && event->x - close_x <= DIMENSIONS && event->y - close_y <= DIMENSIONS)
            return 0;
        else
            printf("TESTING: event %i,%i close %i,%i\n", event->x, event->y, close_x, close_y);
        return 1;
    }
}

void Athena_DefaultTechOverlayDraw(const struct Athena_ButtonArgList *arg, struct Athena_Image *framebuffer){
    struct Athena_Viewport p = {NULL, MARGINS, MARGINS, 0, 0};
    p.image = framebuffer;
    p.w = framebuffer->w - (MARGINS<<1);
    p.h = framebuffer->h - (MARGINS<<1);
    Athena_DrawDefaultWindowStyle(&p);

    Athena_ShrinkViewport(&p, 4, 4, 4, 4);
    athena_draw_tech_tree_iter(&p, Athena_GetDefaultTechTree()->bonuses);

    { /* Lastly draw the close button. */
        unsigned lx, ly;
        struct Athena_Viewport close_box;
        close_box.image = framebuffer;
        athena_tech_overlay_close_xy(framebuffer, &lx, &ly);
        close_box.x = lx;
        close_box.y = ly;
        close_box.w = close_box.h = DIMENSIONS;

        Athena_DrawDefaultWindowStyle(&close_box);
        Athena_BlendViewport(&close_box, Athena_RGBAToRaw(0xFF, 0, 0, 0xFF), Athena_RGBARawMultiply);
    }
}

int Athena_LoadTechTreeFromFile(const char *file, struct Athena_TechTree *to){
    int size;
    void * const data = BufferFile(file, &size);
    
    if(data){
        const int r = Athena_LoadTechTreeFromMemory(data, size, to);
        FreeBufferFile(data, size);
        return r;
    }
    else
        return -1;
}

int Athena_LoadTechTreeFromMemory(const void *data, unsigned len, struct Athena_TechTree *to){
    struct Turbo_Value value;
    const char * const source = data;
    Turbo_Value(&value, source, source + len);
    if(value.type==TJ_Object)
        return Athena_LoadTechTreeFromTurboValue(&value, to);
    Turbo_FreeParse(&value);
    return -2;
}

static int athena_tech_bonus_iter(const struct Turbo_Value *from, unsigned n, struct Athena_BonusList **bonuses){
    if(from->type!=TJ_Object){
        if(from->type!=TJ_String)
            return -101;
        else{
            Athena_AppendBonusN(from->value.string, from->length, ATHENA_NON_BONUS, bonuses);
            return athena_tech_bonus_iter(from+1, n-1, &(bonuses[0]->next));
        }
    }
    else{
        const struct Turbo_Value 
            * const what = Turbo_Helper_GetConstObjectElement(from, "what"),
            * const amount = Turbo_Helper_GetConstObjectElement(from, "amount");
        if(!what || !amount || what->type!=TJ_String || amount->type!=TJ_Number)
            return -102;
        else{
            Athena_AppendBonusN(what->value.string, what->length, amount->value.number, bonuses);
            return athena_tech_bonus_iter(from+1, n-1, &(bonuses[0]->next));
        }
    }
}

int Athena_LoadTechTreeFromTurboValue(const struct Turbo_Value *value, struct Athena_TechTree *to){
    if(value->type==TJ_Object){
        const struct Turbo_Value 
            * const bonuses = Turbo_Helper_GetConstObjectElement(value, "bonuses"),
            * const clazzes = Turbo_Helper_GetConstObjectElement(value, "clazzes");
        if(!bonuses || !clazzes || bonuses->type!=TJ_Array || clazzes->type!=TJ_Array )
            return -4;
        memset(to, 0, sizeof(struct Athena_TechTree));
        return athena_tech_bonus_iter(bonuses->value.array, bonuses->length, &to->bonuses);
    }
    else
        return -3;
}

static void athena_draw_tech_tree_iter(struct Athena_Viewport *to, struct Athena_BonusList *bonuses){
    if(bonuses){
        WriteString(GetTitleFont(), bonuses->title, to->image, to->x + 2, to->y + 2);
        {
            char buffer[100];
            int amount = bonuses->bonus.amount;
            char prefix = '+';
            if(bonuses->bonus.amount<0){
                amount=-amount;
                prefix = '-';
            }
            sprintf(buffer, "%s %c%i", bonuses->bonus.what, prefix, amount);

            WriteString(GetSystemFont(), buffer, to->image, to->x + 2, to->y + 12);
        }
        Athena_ShrinkViewport(to, 0, 24, 0, 0);
        athena_draw_tech_tree_iter(to, bonuses->next);
    }
}

void Athena_DrawTechTree(struct Athena_TechTree *tree, struct Athena_Viewport *to){
    struct Athena_Viewport p = *to;
    Athena_DrawDefaultWindowStyle(to);
    Athena_ShrinkViewport(&p, 4, 4, 4, 4);
    athena_draw_tech_tree_iter(&p, tree->bonuses);
}

void Athena_AppendBonus(const char *what, int amount, struct Athena_BonusList **to){
    Athena_AppendBonusN(what, strlen(what), amount, to);
}

void Athena_AppendBonusN(const char *what, unsigned len, int amount, struct Athena_BonusList **to){
    if(to[0]!=NULL)
        Athena_AppendBonus(what, amount, &to[0]->next);
    else{
        struct Athena_BonusList *const l = to[0] = malloc(sizeof(struct Athena_BonusList));
        if(len>=80)
            len = 79;
        memcpy(l->bonus.what, what, len);
        l->bonus.what[len] = 0;
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
