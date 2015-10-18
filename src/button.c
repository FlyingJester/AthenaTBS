#include "button.h"
#include "image.h"
#include "window_style.h"
#include "font.h"

int Athena_UIDrawButtons(struct Athena_ButtonList *buttons, struct Athena_Viewport *onto){
    if(!buttons){
        return 0;
    }
    else{
        onto->x = buttons->button.x;
        onto->y = buttons->button.y;
        onto->w = buttons->button.w;
        onto->h = buttons->button.h;
        Athena_DrawDefaultWindowStyle(onto);
        WriteString(GetSystemFont(), buttons->button.text, onto->image, buttons->button.x + 2, buttons->button.y + 2);

        if(buttons->button.clicked){
            buttons->button.clicked--;
            Athena_BlendViewport(onto, Athena_RGBAToRaw(0xFF>>1, 0xFF>>1, 0xFF>>1, 0xFF), Athena_RGBARawMultiply);
        }

        return Athena_UIDrawButtons(buttons->next, onto);
    }
}
