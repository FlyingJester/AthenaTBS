#pragma once
#include "image.h"

/* Circularly linked... */
struct Athena_AnimationFrame {
    unsigned time;
    struct Athena_MaskedImage frame;
    struct Athena_AnimationFrame *next;
};

struct Athena_Animation {
    unsigned long last_time;
    /* frames _must_ be a circular linked list. */
    const struct Athena_AnimationFrame *frames;
};

int Athena_AnimationTick(struct Athena_Animation *animation);
int Athena_DrawAnimation(const struct Athena_Animation *animation, struct Athena_Image *onto, int x, int y);
int Athena_DrawAnimationBlendMode(const struct Athena_Animation *animation, struct Athena_Image *onto, int x, int y,
    uint32_t(*blend_func)(uint32_t, uint32_t));
int Athena_DrawAnimationMask(const struct Athena_Animation *animation, struct Athena_Image *onto, int x, int y, uint32_t color);
