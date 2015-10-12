#include "button.h"

static int athena_set_menu_position(struct Athena_ButtonList *buttons, int x, int last_y, int w, int h){
    if(!buttons){
        return 0;
    }
    else{
        buttons->button.x = x;
        buttons->button.y = last_y;
        buttons->button.w = w;
        buttons->button.h = h;
        return athena_set_menu_position(buttons->next, x, last_y + h, w, h);
    }
}

void Athena_OrganizeMenu(struct Athena_Menu *menu){
    athena_set_menu_position(menu->buttons, menu->x + 4, menu->y + 4, menu->w - 8, 20);
}
