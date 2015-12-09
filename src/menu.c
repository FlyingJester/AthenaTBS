#include "menu.h"
#include "font.h"
#include "window_style.h"

static int athena_set_menu_position(struct Athena_ButtonList *buttons, int x, int last_y, int w, int h){
    if(!buttons){
        return last_y + 4;
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
    menu->h = athena_set_menu_position(menu->buttons, menu->x + 4, menu->y + 20, menu->w - 8, 20) - menu->y;
}

static struct Athena_Viewport *athena_viewport_from_menu(const struct Athena_Menu *from, struct Athena_Image *image, struct Athena_Viewport *to){
    Athena_ViewportFromMenu(from, image, to);
    return to;
}

void Athena_DrawMenu(struct Athena_Menu *menu, struct Athena_Viewport *onto){
    struct Athena_Viewport frame;

    Athena_DrawDefaultWindowStyle(athena_viewport_from_menu(menu, onto->image, &frame));
    WriteString(GetTitleFont(), menu->text, onto->image, menu->x + 4, menu->y + 4);
    Athena_UIDrawButtons(menu->buttons, onto);
}

void Athena_ViewportFromMenu(const struct Athena_Menu *from, struct Athena_Image *image, struct Athena_Viewport *to){
    to->image = image;
    to->x = from->x;
    to->y = from->y;
    to->w = from->w;
    to->h = from->h;
}
