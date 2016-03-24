#include "player.h"
#include "unit.h"
#include "image.h"
#include "font.h"
#include "window_style.h"
#include <stdio.h>
#include <string.h>

static unsigned athena_builds(unsigned remaining, struct Athena_Class **buildable,
    unsigned i, const char *builder, const struct Athena_Class **out){
    if(remaining == 0){
        return i;
    }
    else{
        if(strcmp(builder, buildable[0]->built_by) == 0){
            out[0] = buildable[0];
            return athena_builds(remaining - 1, buildable + 1, i + 1, builder, out + 1);
        }
        else
            return athena_builds(remaining - 1, buildable + 1, i, builder, out);
    }
}

static unsigned athena_can_build(unsigned n, struct Athena_Class **const buildable, const struct Athena_Class *class){
    if(n == 0)
        return 0;
    else{
        
        if(*buildable == class)
            return 1;
        else
            return athena_can_build(n-1, buildable+1, class);
    }
}

unsigned Athena_CanBuild(const struct Athena_Player *player, struct Athena_Class *class){
    return athena_can_build(player->num_buildable, player->buildable, class);
}

/* Will return at most 16. Returns the number of matches filled in. */
unsigned Athena_GetBuilds(const struct Athena_Player *player, const char *builder, const struct Athena_Class **out){
    return athena_builds(player->num_buildable, player->buildable, 0, builder, out);
}

void Athena_DrawPlayerDataBox(const struct Athena_Player *player, struct Athena_Viewport *to){
    
    struct Athena_WindowStyle style;
    Athena_DefaultWindowStyle(&style);

    style.mask = player->color;

    Athena_DrawWindowStyle(&style, to);

    WriteString(GetTitleFont(), player->name, to->image, to->x + 20, to->y + 4);
    
    if(player->flag.pixels){
        Athena_BlitBlended(&player->flag, to->image, to->x + 4, to->y + 4);
    }
    
    {
        char buffer[0x100];
        sprintf(buffer, "%i", player->resources.cash);
        WriteString(GetSystemFont(), "Cash", to->image, to->x + 20, to->y + 20);
        WriteString(GetSystemFont(), buffer, to->image, to->x + 70, to->y + 20);
        sprintf(buffer, "%i", player->resources.metal);
        WriteString(GetSystemFont(), "Metal",to->image, to->x + 20, to->y + 34);
        WriteString(GetSystemFont(), buffer, to->image, to->x + 70, to->y + 34);
        sprintf(buffer, "%i", player->resources.food);
        WriteString(GetSystemFont(), "Food", to->image, to->x + 20, to->y + 48);
        WriteString(GetSystemFont(), buffer, to->image, to->x + 70, to->y + 48);
    }
}

unsigned Athena_CanPayResources(const struct Athena_Player *player, const struct Athena_Resources *amount){
    return
        player->resources.cash >= amount->cash &&
        player->resources.metal >= amount->metal &&
        player->resources.food >= amount->food;
}

unsigned Athena_PayResources(struct Athena_Player *player, const struct Athena_Resources *amount){
    const unsigned worked = Athena_CanPayResources(player, amount);
    Athena_SubResources(&player->resources, amount);
    return worked;
}

static void athena_accumulate_unit_income(const struct Athena_Player *player, const struct Athena_UnitList *list, struct Athena_Resources *to){
    if(list){
        if(list->unit.owner==player)
            Athena_AddResources(to, &list->unit.clazz->income);
        athena_accumulate_unit_income(player, list->next, to);
    }
}

void Athena_GivePlayerIncome(struct Athena_Player *player, const struct Athena_UnitList *list){
    struct Athena_Resources amount;
    Athena_PlayerIncome(player, list, &amount);
    Athena_AddResources(&player->resources, &amount);
}

void Athena_PlayerIncome(const struct Athena_Player *player, const struct Athena_UnitList *list, struct Athena_Resources *to){
    memset(to, 0, sizeof(struct Athena_Resources));
    athena_accumulate_unit_income(player, list, to);
}

static const char *athena_find_tech(const char *which, const char **has_tech, unsigned num_has_tech){
    if(!num_has_tech)
        return NULL;
    else if(strcmp(*has_tech, which)==0){
        return *has_tech;
    }   
    else
        return athena_find_tech(which, has_tech+1, num_has_tech-1);
}

static unsigned athena_meets_bonus_reqs(const char **has_tech, unsigned num_has_tech, const char **needs_tech, unsigned num_needs_tech){
    if(num_needs_tech==0)
        return 1;
    else{
        const char * const found = athena_find_tech(*needs_tech, has_tech, num_needs_tech);
        if(!found)
            return 0;
        else if(found==*has_tech){
            has_tech++;
            num_has_tech--;
        }
        else if(0 && found==has_tech[num_has_tech-1]){
            num_has_tech--;
        }

        return athena_meets_bonus_reqs(has_tech, num_has_tech, needs_tech+1, num_needs_tech-1);
    }
}

unsigned Athena_PlayerMeetsReqsBonus(const struct Athena_Player *player, const struct Athena_BonusList *bonus){
    return athena_meets_bonus_reqs(player->tech, player->num_tech, bonus->reqs, bonus->num_reqs);
}
