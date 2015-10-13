#pragma once
#include "image.h"

struct Athena_AnimationFrame {
    unsigned time;
    struct Athena_Image *frame;
    struct Athena_AnimationFrame *next;
};

struct Athena_Animation {
    unsigned time;
    /* frames _must_ be a circular linked list. */
    struct Athena_AnimationFrame *frames;
};

void Athena_AnimationTick(struct Athena_Animation *animation, unsigned ticks);
void Athena_DrawAnimation(const struct Athena_Animation *animation, struct Athena_Image *onto, int x, int y);
