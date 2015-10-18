#pragma once
#include "button.h"

struct Athena_Menu{
    struct Athena_ButtonList *buttons;
    const char *text;
    int x, y;
    unsigned w, h;
};

void Athena_OrganizeMenu(struct Athena_Menu *menu);
void Athena_DrawMenu(struct Athena_Menu *menu, struct Athena_Viewport *onto);
void Athena_ViewportFromMenu(const struct Athena_Menu *from, struct Athena_Image *image, struct Athena_Viewport *to);
