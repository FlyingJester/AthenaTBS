#include "player.h"
#include "image.h"
#include "font.h"
#include "window_style.h"

void Athena_DrawPlayerDataBox(const struct Athena_Player *player, struct Athena_Viewport *to){
    
    struct Athena_WindowStyle style;
    Athena_DefaultWindowStyle(&style);

    style.mask = player->color;

    Athena_DrawWindowStyle(&style, to);

    WriteString(GetSystemFont(), player->name, to->image, to->x + 20, to->y + 4);
    
    if(player->flag.pixels){
        Athena_BlitBlended(&player->flag, to->image, to->x + 4, to->y + 4);
    }
    
    {
        
    }
}
