#include "spriteset.h"
#include "container.h"
#include <TurboJSON/parse.h>
#include <TurboJSON/object.h>
#include "turbo_json_helpers.h"
#include "bufferfile/bufferfile.h"
#include "path/path.h"
#include <stdlib.h>
#include <string.h>

unsigned Athena_AddImageSpriteset(struct Athena_Spriteset *ss, struct Athena_Image *image){
    ss->images = Athena_AssureCapacity(ss->images, sizeof(struct Athena_Image), ss->num_images+1, &(ss->images_capacity));
    ss->images[ss->num_images++] = *image;
    return ss->num_images-1;
}

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

int Athena_LoadSpritesetFromFile(const char *file, struct Athena_Spriteset *to){
    int size, ret = -2;
    void * const data = BufferFile(file, &size);
    
    if(data){
        char * const directory = Athena_GetContainingDirectory(file);
        ret = Athena_LoadSpritesetFromMemory(data, size, to, directory);
        free(directory);
        FreeBufferFile(data, size);
    }
    
    return ret;
}

int Athena_LoadSpritesetFromMemory(const void *data, unsigned len, struct Athena_Spriteset *to, const char *directory){
    struct Turbo_Value value;
    const char * const source = data;
    Turbo_Value(&value, source, source + len);
    if(value.type==TJ_Object)
        return Athena_LoadSpritesetFromTurboValue(&value, to, directory);
    Turbo_FreeParse(&value);
    return -2015;
}

/*
Example spriteset:
{
    "actions":{
        "building":{
            "south":[
                {"delay":1, "image":0}
            ]
        },
        "idle":{
            "south":[
                {"delay":1, "image":0}
            ]
        }
    },
    "images":["Fortification.png"]
}
*/

static int athena_load_spriteset_images(const struct Turbo_Value *val_array, unsigned long len, struct Athena_Image *images, const char *directory){
    if(!len)
        return 0;
    else{
        const unsigned dir_length = strlen(directory);
        char * const filename = malloc(val_array->length + dir_length + 2);

        memcpy(filename, directory, dir_length);
        memcpy(filename + dir_length + 1, val_array->value.string, val_array->length);
        filename[dir_length] = '/';
        filename[dir_length + 1 + val_array->length] = '\0';

        {
            const int err = Athena_LoadAuto(images, filename);
            free(filename);
            if(err!=0){
                /* Do you remember Tetris for the Gameboy? Remember the sound it made 
                 * when you lost and it filled the screen with little X's?
                 * Imagine that sound when this line is executed:
                 */
                memset(images, 0, len * sizeof(struct Athena_Image));
                return -17;
            }
        }
        return athena_load_spriteset_images(val_array + 1, len - 1, images + 1, directory);
    }
}

/*
struct Athena_SpriteAction {
    const char *name;
    struct Athena_SpriteDirection *directions;
    unsigned num_directions, directions_capacity;
};
*/

static struct Athena_AnimationFrame *athena_last_spriteset_frame(struct Athena_AnimationFrame *frame){
    if(!frame)
        return NULL;
    else if(frame->next)
        return athena_last_spriteset_frame(frame->next);
    else 
        return frame;
}

static int athena_load_spriteset_frame(const struct Turbo_Value *properties, unsigned long num_properties,
    struct Athena_Image *images, struct Athena_AnimationFrame **frame_p){
    if(!num_properties){
        frame_p[0] = NULL;
        return 0;
    }
    else{
        const struct Turbo_Value 
            * const image = Turbo_Helper_GetConstObjectElement(properties, "image"),
            * const delay = Turbo_Helper_GetConstObjectElement(properties, "delay");
        if(!(image && delay))
            return -131;
        else if(image->type != TJ_Number)
            return -132;
        else if(delay->type != TJ_Number)
            return -132;
        else{
            /*
            struct Athena_AnimationFrame {
                unsigned time;
                struct Athena_Image *frame;
                struct Athena_AnimationFrame *next;
            };
            */
        
            struct Athena_AnimationFrame * const frame = frame_p[0] = malloc(sizeof(struct Athena_AnimationFrame));
            frame->time = (int)delay->value.number;
            frame->frame = images + (int)image->value.number;
            
            return athena_load_spriteset_frame(properties + 1, num_properties - 1, images, &frame->next);
        }
    }
}

static int athena_load_spriteset_direction(const struct Turbo_Property *direction_values, unsigned long num_directions, 
    struct Athena_Image *images, struct Athena_SpriteDirection *directions){

    if(!num_directions)
        return 0;
    else if(direction_values->value.type != TJ_Array)
        return -111;
    else if(!direction_values->value.length)
        return -112;
    else{
        
        char *direction_name = malloc(direction_values->name_length + 1);
        memcpy(direction_name, direction_values->name, direction_values->name_length);
        direction_name[direction_values->name_length] = '\0';
        
        directions->name = direction_name;

        athena_load_spriteset_frame(direction_values->value.value.array, direction_values->value.length, images, &directions->frames);
        athena_last_spriteset_frame(directions->frames)->next = directions->frames;

        return athena_load_spriteset_direction(direction_values + 1, num_directions - 1, images, directions + 1);
    }
}

static int athena_load_spriteset_action(const struct Turbo_Property *action_values, unsigned long num_actions, 
    struct Athena_Image *images, struct Athena_SpriteAction *actions){

    if(!num_actions)
        return 0;
    else if(action_values->value.type != TJ_Object)
        return -101;
    else{

        char *action_name = malloc(action_values->name_length + 1);
        memcpy(action_name, action_values->name, action_values->name_length);
        action_name[action_values->name_length] = '\0';
        
        actions->name = action_name;
        
        actions->num_directions = action_values->value.length;
        actions->directions_capacity = 0;

        actions->directions = Athena_AssureCapacity(NULL, sizeof(struct Athena_SpriteDirection), actions->num_directions, &actions->directions_capacity);
        
        {
            const int err = athena_load_spriteset_direction(action_values->value.value.object, action_values->value.length, images, actions->directions);
            if (err!=0)
                return err;
            else
                return athena_load_spriteset_action(action_values + 1, num_actions - 1, images, actions + 1);
        }
    }
}

/* value->type _must_ be Object */
int Athena_LoadSpritesetFromTurboValue(const struct Turbo_Value *value, struct Athena_Spriteset *to, const char *directory){
    const struct Turbo_Value 
        * const images = Turbo_Helper_GetConstObjectElement(value, "images"),
        * const actions = Turbo_Helper_GetConstObjectElement(value, "actions");
    
    if(!(images && actions))
        return -1;
    
    to->num_actions = actions->length;
    to->actions_capacity = 0;
    to->actions = Athena_AssureCapacity(NULL, sizeof(struct Athena_SpriteAction), to->num_actions, &to->actions_capacity);

    to->num_images = images->length;
    to->images_capacity = 0;
    to->images = Athena_AssureCapacity(NULL, sizeof(struct Athena_Image), to->num_images, &to->images_capacity);

    athena_load_spriteset_images(images->value.array, images->length, to->images, directory);
    athena_load_spriteset_action(actions->value.object,actions->length,to->images, to->actions);
    
    return 0;
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
