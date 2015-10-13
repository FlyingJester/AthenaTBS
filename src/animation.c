#include "animation.h"

void Athena_AnimationTick(struct Athena_Animation *animation, unsigned ticks){
    animation->time += ticks;
    if(animation->time > animation->frames->time){
        animation->frames = animation->frames->next;
        animation->time = 0;
    }
}

void Athena_DrawAnimation(const struct Athena_Animation *animation, struct Athena_Image *onto, int x, int y){
    if(animation->frames->frame)
        Athena_Blit(animation->frames->frame, onto, x, y);
}
