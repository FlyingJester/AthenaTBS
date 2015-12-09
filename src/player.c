#include "player.h"
#include "image.h"
#include "font.h"
#include "window_style.h"
#include <stdio.h>

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

unsigned Athena_CanPayResources(const struct Athena_Resources *payer, const struct Athena_Resources *amount){
    return
        payer->cash >= amount->cash &&
        payer->metal >= amount->metal &&
        payer->food >= amount->food;
}

unsigned Athena_PayResources(struct Athena_Resources *payer, const struct Athena_Resources *amount){
    const unsigned worked = Athena_CanPayResources(payer, amount);
    
    payer->cash -= amount->cash;
    payer->metal -= amount->metal;
    payer->food -= amount->food;
    
    return worked;
}
