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

int Athena_DrawAnimation(const struct Athena_Animation *animation, struct Athena_Image *image, int x, int y){
    if(!(animation && image))
        return 1;
    else{
        Athena_BlitBlended(animation->frames->frame, image, x, y);
        return 0;
    }
}
