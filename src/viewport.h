#pragma once
#include "image.h"

struct Athena_Viewport{
    struct Athena_Image *image;
    int x, y;
    unsigned w, h;
};

/* Increases port->x by `left`, while ensuring that port->x + port->w stays the same
 * Increases port->y by `top`, while ensuring that port->y + port->h stays the same
 * Increases port->x + port->w by `right`, while ensuring that port->x stays the same
 * Increases port->y + port->h by `bottom`, while ensuring that port->y stays the same
 */

void Athena_ShrinkViewport(struct Athena_Viewport *port, int left, int top, int right, int bottom);

unsigned Athena_IsWithinViewport(const struct Athena_Viewport *port, int x, int y);
unsigned Athena_IsWithinXYWH(int p_x, int p_y, unsigned p_w, unsigned p_h, int x, int y);
#define Athena_IsWithin(THAT_, X_, Y_)\
    Athena_IsWithinXYWH((THAT_).x, (THAT_).y, (THAT_).w, (THAT_).h, X_, Y_)
