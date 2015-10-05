#include "spriteset.h"
#include <stdlib.h>
#include <string.h>

const struct Athena_AnimationFrame *athena_get_matching_direction(
    const struct Athena_SpriteAction *action, const char *direction, unsigned i){

    if(i<action->num_directions){
        if(strcmp(direction, action->directions[i].name)==0){
            return action->directions[i].frames;
        }
        else{
            return athena_get_matching_direction(action, direction, i+1);
        }
    }
    else{
        return NULL;
    }
}

unsigned athena_string_contains(const char *needle, const char *haystack){
    const unsigned haystack_len = strlen(haystack), needle_len = strlen(needle);
    unsigned i = 0;
    
    while(needle_len + i <= haystack_len){
        if(memcmp(needle, haystack + i, needle_len)==0)
            return 1;
        i++;
    }
    return 0;
}

const struct Athena_AnimationFrame *athena_get_partial_direction(
    const struct Athena_SpriteAction *action, const char *direction, unsigned i){

    if(i<action->num_directions){
        if(athena_string_contains(direction, action->directions[i].name)){
            return action->directions[i].frames;
        }
        else{
            return athena_get_partial_direction(action, direction, i+1);
        }
    }
    else{
        return NULL;
    }
}

const struct Athena_SpriteAction *athena_get_matching_action(
    struct Athena_SpriteAction *actions, unsigned num_actions, const char *name){
    if(!num_actions)
        return NULL;
    else if(strcmp(actions->name, name)==0)
        return actions;
    else
        return athena_get_matching_action(actions + 1, num_actions - 1, name);
}

const struct Athena_AnimationFrame *Athena_GetSpritesetDirection(
    const struct Athena_Spriteset *spriteset,
    const char *action_name, const char *direction_name, unsigned *diag){

    unsigned diag_dummy;
    unsigned * const diag_p = (diag)?diag:(&diag_dummy);
    const struct Athena_SpriteAction *action;
    
    if(spriteset->actions == NULL || spriteset->num_actions==0)
        return NULL;

    if((action = athena_get_matching_action(spriteset->actions, spriteset->num_actions, action_name))){
        const struct Athena_AnimationFrame *frame;
        if((frame = athena_get_matching_direction(action, direction_name, 0))){
            diag_p[0] = ATHENA_ANIMATION_EXACT_MATCH;
            return frame;
        }
        if((frame = athena_get_partial_direction(action, direction_name, 0))){
            diag_p[0] = ATHENA_ANIMATION_ACTION_MATCH_DIRECTION_PARTIAL;
            return frame;
        }
        diag_p[0] = ATHENA_ANIMATION_ACTION_MATCH_DIRECTION_WRONG;
        return action->directions->frames;
    }
    else{
        diag_p[0] = ATHENA_ANIMATION_NO_MATCH;
        return spriteset->actions->directions->frames;
    }

}

/* ========================================================================== */
/* Spriteset Tests */
/* ========================================================================== */

int Athena_Test_DirectionMatching(){
    struct Athena_SpriteDirection test_direction = { "test_direction", (void *)(0xDEADBEE) };
    struct Athena_SpriteAction test_action = { NULL, NULL, 1, 1 };

    test_action.directions = &test_direction;
    return athena_get_matching_direction(&test_action, "test_direction", 0) == (void *)(0xDEADBEE);
}

#define ATHENA_TEST_SINGLE_DIRECTION_PARTIAL(NAME, NEEDLE, HAYSTACK, EQUAL)\
int Athena_Test_DirectionPartial ## NAME(){\
    struct Athena_SpriteDirection test_direction = { HAYSTACK, (void *)(0xDEADBEE) };\
    struct Athena_SpriteAction test_action = { NULL, NULL, 1, 1 };\
    test_action.directions = &test_direction;\
    {\
        const unsigned success =\
            athena_get_partial_direction(&test_action, NEEDLE, 0) == (void *)(0xDEADBEE);\
        if(EQUAL) return success; else return !success;\
    }\
}

ATHENA_TEST_SINGLE_DIRECTION_PARTIAL(Matches, "test_direction", "test_direction", 1)
ATHENA_TEST_SINGLE_DIRECTION_PARTIAL(NotMatches, "lorem_ipsum", "test_direction", 0)
ATHENA_TEST_SINGLE_DIRECTION_PARTIAL(StartsWith, "test", "test_direction", 1)
ATHENA_TEST_SINGLE_DIRECTION_PARTIAL(EndsWith, "direction", "test_direction", 1)
ATHENA_TEST_SINGLE_DIRECTION_PARTIAL(Contains, "st_dire", "test_direction", 1)

int Athena_Test_GetMatchingDirectionSingle(){
    struct Athena_SpriteDirection test_direction = {"test_direction", (void *)(0xDEADBEE)};
    struct Athena_SpriteAction test_action = {"test_action", NULL, 1};
    struct Athena_Spriteset test_spriteset = {NULL, 1, 1};
    
    unsigned diag;

    test_action.directions = &test_direction;
    test_spriteset.actions = &test_action;
    
    Athena_GetSpritesetDirection(&test_spriteset, "test_action", "test_direction", &diag);

    return diag == ATHENA_ANIMATION_EXACT_MATCH;
}

int Athena_Test_GetMatchingDirectionMultiple(){
    struct Athena_SpriteDirection test_directions[] = {
        {"test_direction1", (void *)(0x0DEADBEE)},
        {"lorem_ipsum", (void *)(0xD0EADBEE)},
        {"i_thought_what_id_do", (void *)(0xDE0ADBEE)},
        {"is_id_pretend_i_was", (void *)(0xDEA0DBEE)},
        {"one_of_those_deaf_mutes", (void *)(0xDEAD0BEE)}
    };
    struct Athena_SpriteAction test_action = {"test_action", NULL, 
        sizeof(test_directions) / sizeof(struct Athena_SpriteDirection)};
    struct Athena_Spriteset test_spriteset = {NULL, 1, 1};
    
    unsigned diag;
    
    test_action.directions = test_directions;
    test_spriteset.actions = &test_action;
    
    return Athena_GetSpritesetDirection(&test_spriteset, "test_action", "lorem_ipsum", &diag) == (void *)(0xD0EADBEE) &&
        diag == ATHENA_ANIMATION_EXACT_MATCH;
}

int Athena_Test_GetSimilarDirectionSingle(){
    struct Athena_SpriteDirection test_direction = {"test_direction", (void *)(0xDEADBEE)};
    struct Athena_SpriteAction test_action = {"test_action", NULL, 1};
    struct Athena_Spriteset test_spriteset = {NULL, 1, 1};
    
    unsigned diag;

    test_action.directions = &test_direction;
    test_spriteset.actions = &test_action;
    
    return Athena_GetSpritesetDirection(&test_spriteset, "test_action", "st_direc", &diag) &&
        diag == ATHENA_ANIMATION_ACTION_MATCH_DIRECTION_PARTIAL;
}

int Athena_Test_GetSimilarDirectionMultiple(){
    struct Athena_SpriteDirection test_directions[] = {
        {"test_direction1", (void *)(0x0DEADBEE)},
        {"lorem_ipsum", (void *)(0xD0EADBEE)},
        {"i_thought_what_id_do", (void *)(0xDE0ADBEE)},
        {"is_id_pretend_i_was", (void *)(0xDEA0DBEE)},
        {"one_of_those_deaf_mutes", (void *)(0xDEAD0BEE)}
    };
    struct Athena_SpriteAction test_action = {"test_action", NULL, 
        sizeof(test_directions) / sizeof(struct Athena_SpriteDirection)};
    struct Athena_Spriteset test_spriteset = {NULL, 1, 1};
    
    unsigned diag;
    
    test_action.directions = test_directions;
    test_spriteset.actions = &test_action;
    
    return Athena_GetSpritesetDirection(&test_spriteset, "test_action", "rem_ips", &diag) == (void *)(0xD0EADBEE) &&
        diag == ATHENA_ANIMATION_ACTION_MATCH_DIRECTION_PARTIAL;
}

static struct Athena_Test athena_tests[] = {
    ATHENA_TEST(Athena_Test_DirectionMatching),
    ATHENA_TEST(Athena_Test_DirectionPartialMatches),
    ATHENA_TEST(Athena_Test_DirectionPartialNotMatches),
    ATHENA_TEST(Athena_Test_DirectionPartialStartsWith),
    ATHENA_TEST(Athena_Test_DirectionPartialEndsWith),
    ATHENA_TEST(Athena_Test_DirectionPartialContains),
    ATHENA_TEST(Athena_Test_GetMatchingDirectionSingle),
    ATHENA_TEST(Athena_Test_GetMatchingDirectionMultiple),
    ATHENA_TEST(Athena_Test_GetSimilarDirectionSingle),
    ATHENA_TEST(Athena_Test_GetSimilarDirectionMultiple)
};

ATHENA_TEST_FUNCTION(Athena_SpritesetTest, athena_tests)
