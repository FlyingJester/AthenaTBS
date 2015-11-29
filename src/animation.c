#include "animation.h"
#include "time/ticks.h"

int Athena_AnimationTick(struct Athena_Animation *animation){
    const unsigned long t = Athena_GetMillisecondTicks();
    if(t - animation->last_time > animation->frames->time){
        animation->frames = animation->frames->next;
        animation->last_time = t;
    }
    return 0;
}


int Athena_DrawAnimationBlendMode(const struct Athena_Animation *animation, struct Athena_Image *image, int x, int y, uint32_t(*blend_func)(uint32_t, uint32_t)){
    if(!(animation && image))
        return 1;
    else{
        Athena_BlitBlendMode(animation->frames->frame, image, x, y, blend_func);
        return 0;
    }
}

int Athena_DrawAnimation(const struct Athena_Animation *animation, struct Athena_Image *image, int x, int y){
    if(!(animation && image))
        return 1;
    else{
        Athena_BlitBlended(animation->frames->frame, image, x, y);
        return 0;
    }
}

int Athena_DrawAnimationMaskBlendMode(const struct Athena_Animation *animation, struct Athena_Image *image, int x, int y,
    uint32_t(*blend_func)(uint32_t, uint32_t), uint32_t color){
    const int err = Athena_DrawAnimationBlendMode(animation, image, x, y, blend_func);

    return err;
}
