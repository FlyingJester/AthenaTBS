#include "player.h"
#include "unit.h"
#include "image.h"
#include "font.h"
#include "window_style.h"
#include <stdio.h>
#include <string.h>

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
        char buffer[80];
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

void Athena_AddResources(struct Athena_Resources *to, const struct Athena_Resources *from){
    to->cash += from->cash;
    to->metal += from->metal;
    to->food += from->food;
}

void Athena_SubResources(struct Athena_Resources *to, const struct Athena_Resources *from){
    to->cash -= from->cash;
    to->metal -= from->metal;
    to->food -= from->food;
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
