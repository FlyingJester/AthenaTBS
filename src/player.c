#include "player.h"
#include "image.h"
#include "font.h"

void Athena_DrawPlayerDataBox(const struct Athena_Player *player, struct Athena_Viewport *to){
    Athena_FillViewport(to, player->color);
    WriteString(GetSystemFont(), player->name, to->image, to->x + 2, to->y + 2);
}
