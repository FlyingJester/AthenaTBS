#pragma once
#include "image.h"

struct Athena_Viewport{
    struct Athena_Image *image;
    int x, y;
    unsigned w, h;
};

unsigned Athena_IsWithinViewport(const struct Athena_Viewport *port, int x, int y);
unsigned Athena_IsWithinXYWH(int p_x, int p_y, unsigned p_w, unsigned p_h, int x, int y);
#define Athena_IsWithin(THAT_, X_, Y_)\
    Athena_IsWithinXYWH((THAT_).x, (THAT_).y, (THAT_).w, (THAT_).h, X_, Y_)
