#pragma once
#include "animation.h"
#include "test.h"

struct Athena_SpriteDirection {
    const char *name;
    struct Athena_AnimationFrame *frames;
};

struct Athena_SpriteAction {
    const char *name;
    struct Athena_SpriteDirection *directions;
    unsigned num_directions, directions_capacity;
};

struct Athena_Spriteset{
    struct Athena_SpriteAction *actions;
    unsigned num_actions, actions_capacity;

    struct Athena_Image *images;
    unsigned num_images, images_capacity;

    struct Athena_Image *masks;
    unsigned num_masks, masks_capacity;
};

struct Athena_Sprite{
    unsigned last_time;
    const struct Athena_AnimationFrame *frames;
};

unsigned Athena_AddImageSpriteset(struct Athena_Spriteset *ss, struct Athena_Image *image);

int Athena_LoadSpritesetFromFile(const char *file, struct Athena_Spriteset *to);
int Athena_LoadSpritesetFromMemory(const void *data, unsigned len, struct Athena_Spriteset *to, const char *directory);

struct Turbo_Value;
/* value->type _must_ be Object */
int Athena_LoadSpritesetFromTurboValue(const struct Turbo_Value *value, struct Athena_Spriteset *to, const char *directory);

#define ATHENA_ANIMATION_EXACT_MATCH 0
#define ATHENA_ANIMATION_ACTION_MATCH_DIRECTION_PARTIAL 1
#define ATHENA_ANIMATION_ACTION_MATCH_DIRECTION_WRONG 2
#define ATHENA_ANIMATION_ACTION_WRONG_DIRECTION_MATCH 4
#define ATHENA_ANIMATION_NO_MATCH 8
/* if diag is not null, it is set to:
 *    0 for an exact match,
 *    1 for a matching action but settling on a lower resolution direction,
 *    2 for a matching action but an unrelated direction,
 *    3 for an unrelated action but a matching direction,
 *    4 for an unrelated action and unrelated direction.
 * Will only return NULL if the spriteset has no actions at all.
 */
const struct Athena_AnimationFrame *Athena_GetSpritesetDirection(
    const struct Athena_Spriteset *spriteset,
    const char *action, const char *direction, unsigned *diag);

int Athena_SpritesetTest();
