#pragma once
#include "image.h"

struct Athena_Viewport{
    struct Athena_Image *image;
    int x, y;
    unsigned w, h;
};
