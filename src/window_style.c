#include "window_style.h"
#include "image.h"

static const unsigned char ul_corner[16] = {
    4, 3, 3, 3,
    3, 4, 4, 4,
    3, 4, 5, 5,
    3, 4, 5, 5
};

static const unsigned char ur_corner[16] = {
    3, 3, 3, 2,
    4, 4, 3, 1,
    5, 3, 2, 1,
    5, 4, 2, 1
};

static const unsigned char dr_corner[16] = {
    4, 3, 2, 1,
    3, 3, 2, 1,
    2, 2, 2, 1,
    1, 1, 1, 0
};

static const unsigned char dl_corner[16] = {
    3, 4, 5, 5,
    3, 4, 3, 4,
    3, 3, 2, 2,
    2, 1, 1, 1
};

void Athena_DefaultWindowStyleCallback(struct Athena_Viewport *to, void *arg, uint32_t mask){
    const uint32_t starting_color = Athena_RGBAToRaw(0, 0, 0, 0xFF), 
        mixing_color = Athena_RGBAToRaw(0xFF, 0xFF, 0xFF, 0x2A);
    /*
     *  EDDD || D || DDDC
     *  DEEE || E || EEDB
     *  DEXX || X || XDCB
     *  DEXX || X || XECB
     * ======OO===OO=====
     *  DEXX ||   || EDCB
     * ======OO===OO=====
     *  DEXX || E || EDCB
     *  DEDE || D || DDCB
     *  DDCC || C || CCCB
     *  CBBB || B || BBBA
     *
     */
    
    struct Athena_Image corner_buffer;
    
    const uint32_t colorA = Athena_RGBARawBlend(mixing_color, starting_color),
        colorB = Athena_RGBARawBlend(mixing_color, colorA),
        colorC = Athena_RGBARawBlend(mixing_color, colorB),
        colorD = Athena_RGBARawBlend(mixing_color, colorC),
        colorE = Athena_RGBARawBlend(mixing_color, colorD),
        colorF = Athena_RGBARawBlend(mixing_color, colorE);
    
    uint32_t colors[6];
    colors[0] = colorA;
    colors[1] = colorB;
    colors[2] = colorC;
    colors[3] = colorD;
    colors[4] = colorE;
    colors[5] = colorF;

    Athena_CreateImage(&corner_buffer, 4, 4);
    /* Draw the corners. */
    Athena_ImageFromPalette(&corner_buffer, ul_corner, colors);
    Athena_Blit(&corner_buffer, to->image, to->x, to->y);

    Athena_ImageFromPalette(&corner_buffer, ur_corner, colors);
    Athena_Blit(&corner_buffer, to->image, to->x + to->w - 5, to->y);

    Athena_ImageFromPalette(&corner_buffer, dr_corner, colors);
    Athena_Blit(&corner_buffer, to->image, to->x + to->w - 5, to->y + to->h - 5);

    Athena_ImageFromPalette(&corner_buffer, dl_corner, colors);
    Athena_Blit(&corner_buffer, to->image, to->x, to->y + to->h - 5);

    /* Draw the sides */
    /* Left */
    Athena_FillRect(to->image, to->x, to->y + 4, 1, to->h - 8,     colorD);
    Athena_FillRect(to->image, to->x + 1, to->y + 4, 1, to->h - 8, colorE);
    Athena_FillRect(to->image, to->x + 2, to->y + 4, 2, to->h - 8, colorF);

    /* Right */
    Athena_FillRect(to->image, to->x + to->w - 1, to->y + 4, 1, to->h - 8, colorA);
    Athena_FillRect(to->image, to->x + to->w - 2, to->y + 4, 1, to->h - 8, colorB);
    Athena_FillRect(to->image, to->x + to->w - 3, to->y + 4, 1, to->h - 8, colorC);
    Athena_FillRect(to->image, to->x + to->w - 4, to->y + 4, 1, to->h - 8, colorD);
    
    /* Top */
    Athena_FillRect(to->image, to->x + 4, to->y,     to->w - 8, 1, colorD);
    Athena_FillRect(to->image, to->x + 4, to->y + 1, to->w - 8, 1, colorE);
    Athena_FillRect(to->image, to->x + 4, to->y + 2, to->w - 8, 2, colorF);

    /* Bottom */
    Athena_FillRect(to->image, to->x + 4, to->y + to->h - 1, to->w - 8, 1, colorA);
    Athena_FillRect(to->image, to->x + 4, to->y + to->h - 2, to->w - 8, 1, colorB);
    Athena_FillRect(to->image, to->x + 4, to->y + to->h - 3, to->w - 8, 1, colorC);
    Athena_FillRect(to->image, to->x + 4, to->y + to->h - 4, to->w - 8, 1, colorD);

    /* Draw the center. */
    Athena_FillRect(to->image, to->x + 4, to->y + 4, to->w - 8, to->h - 8, colorF);

    Athena_BlendViewport(to, mask, Athena_RGBARawMultiply);
}

void Athena_DrawDefaultWindowStyle(struct Athena_Viewport *onto){
    struct Athena_WindowStyle style;
    
    Athena_DefaultWindowStyle(&style);
    
    Athena_DrawWindowStyle(&style, onto);
}

void Athena_DefaultWindowStyle(struct Athena_WindowStyle *into){
    into->callback = Athena_DefaultWindowStyleCallback;
    into->mask = 0xFFFFFFFF;
}

void Athena_DrawWindowStyle(struct Athena_WindowStyle *style, struct Athena_Viewport *onto){
    if(style->callback)
        style->callback(onto, style->arg, style->mask);
}
