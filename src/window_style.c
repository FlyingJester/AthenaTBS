#include "window_style.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static void athena_draw_window_style_iter(int x, struct Athena_WindowStyle *style, struct Athena_Viewport *onto){
    if(x--){
        athena_window_style_callback const callback = style->callbacks[x];
        if(callback)
            callback(onto, style->arg);
        athena_draw_window_style_iter(x, style, onto);
    }
}

void Athena_DrawWindowStyle(struct Athena_WindowStyle *style, struct Athena_Viewport *onto){
    struct Athena_Viewport draw = *onto;
    athena_draw_window_style_iter(ATHENA_WINDOWSTYLE_N_CALLBACKS, style, &draw);
}

static const uint32_t original_palette[4] = {
    0xFF505050, /* Fully masked */
    0xFF605050, /* This is only 1/8th masked */
    0xFF301010, /* This is only 1/8th masked */
    0xFF808080  /* Fully masked */
};

#define ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(ARG, PALETTE)\
    uint32_t PALETTE ## TEMP[sizeof(original_palette) >> 2];\
    const uint32_t *PALETTE = PALETTE ## TEMP;\
    if(ARG){\
        const unsigned char *OLD = (void *)original_palette;\
        unsigned char *NEW = (void *)PALETTE ## TEMP;\
        unsigned i = 0;\
        const uint32_t arg_z_ = (unsigned)ARG;\
        const unsigned char *arg_as_uc = (void *)&arg_z_;\
        while(i < sizeof(palette)){\
            const unsigned short c = OLD[i] + arg_as_uc[i % 4];\
            NEW[i] = c>>1;\
            i++;\
        }\
    }\
    else\
        PALETTE = original_palette;

static void athena_default_draw_ul(struct Athena_Viewport *onto, void *arg){
    ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(arg, palette);
    
    Athena_SetPixel(onto->image, onto->x,     onto->y, palette[0]);
    Athena_SetPixel(onto->image, onto->x + 1, onto->y, palette[1]);
    Athena_SetPixel(onto->image, onto->x,     onto->y+1, palette[1]);
    Athena_SetPixel(onto->image, onto->x + 1, onto->y+1, palette[0]);

}

static void athena_default_draw_up(struct Athena_Viewport *onto, void *arg){
    ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(arg, palette);

    Athena_FillRect(onto->image, onto->x + 2, onto->y,     onto->w - 4, 1, palette[1]);
    Athena_FillRect(onto->image, onto->x + 2, onto->y + 1, onto->w - 4, 1, palette[0]);
}

static void athena_default_draw_ur(struct Athena_Viewport *onto, void *arg){
    ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(arg, palette);

    Athena_SetPixel(onto->image, onto->x + onto->w - 1, onto->y,     palette[0]);
    Athena_SetPixel(onto->image, onto->x + onto->w - 2, onto->y,     palette[1]);
    Athena_SetPixel(onto->image, onto->x + onto->w - 1, onto->y + 1, palette[2]);
    Athena_SetPixel(onto->image, onto->x + onto->w - 2, onto->y + 1, palette[0]);
}

static void athena_default_draw_rt(struct Athena_Viewport *onto, void *arg){
    ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(arg, palette);

    Athena_FillRect(onto->image, onto->x + onto->w - 1, onto->y + 2, 1, onto->h - 4, palette[1]);
    Athena_FillRect(onto->image, onto->x + onto->w - 2, onto->y + 2, 1, onto->h - 4, palette[0]);
}

static void athena_default_draw_dr(struct Athena_Viewport *onto, void *arg){
    ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(arg, palette);

    Athena_SetPixel(onto->image, onto->x + onto->w - 1, onto->y + onto->h - 1, palette[0]);
    Athena_SetPixel(onto->image, onto->x + onto->w - 2, onto->y + onto->h - 1, palette[2]);
    Athena_SetPixel(onto->image, onto->x + onto->w - 1, onto->y + onto->h - 2, palette[2]);
    Athena_SetPixel(onto->image, onto->x + onto->w - 2, onto->y + onto->h - 2, palette[0]);
}

static void athena_default_draw_dn(struct Athena_Viewport *onto, void *arg){
    ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(arg, palette);

    Athena_FillRect(onto->image, onto->x + 2, onto->y + onto->h - 1, onto->w - 4, 1, palette[2]);
    Athena_FillRect(onto->image, onto->x + 2, onto->y + onto->h - 2, onto->w - 4, 1, palette[0]);
}

static void athena_default_draw_dl(struct Athena_Viewport *onto, void *arg){
    ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(arg, palette);

    Athena_SetPixel(onto->image, onto->x,     onto->y + onto->h - 1, palette[0]);
    Athena_SetPixel(onto->image, onto->x + 1, onto->y + onto->h - 1, palette[2]);
    Athena_SetPixel(onto->image, onto->x,     onto->y + onto->h - 2, palette[1]);
    Athena_SetPixel(onto->image, onto->x + 1, onto->y + onto->h - 2, palette[0]);
}

static void athena_default_draw_lf(struct Athena_Viewport *onto, void *arg){
    ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(arg, palette);

    Athena_FillRect(onto->image, onto->x,     onto->y + 2, 1, onto->h - 4, palette[1]);
    Athena_FillRect(onto->image, onto->x + 1, onto->y + 2, 1, onto->h - 4, palette[0]);
}

static void athena_default_draw_bg(struct Athena_Viewport *onto, void *arg){
    ATHENA_DEFAULT_WINDOWSYLE_PROLOGUE(arg, palette);

    Athena_FillRect(onto->image, onto->x + 2, onto->y + 2, onto->w - 4, onto->h - 4, palette[3]);
}

static athena_window_style_callback athena_default_callbacks[ATHENA_WINDOWSTYLE_N_CALLBACKS] = {
    athena_default_draw_ul,
    athena_default_draw_up,
    athena_default_draw_ur,
    athena_default_draw_rt,
    athena_default_draw_dr,
    athena_default_draw_dn,
    athena_default_draw_dl,
    athena_default_draw_lf,
    athena_default_draw_bg
};

void Athena_DefaultWindowStyle(struct Athena_WindowStyle *into){
    memcpy(into->callbacks, athena_default_callbacks,
    sizeof(athena_window_style_callback) * ATHENA_WINDOWSTYLE_N_CALLBACKS);
}
