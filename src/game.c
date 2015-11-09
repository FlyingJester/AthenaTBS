#include "game.h"
#include "unit.h"
#include "ui_thread.h"
#include "server_thread.h"
#include "thread/thread.h"
#include "time/sleep.h"
#include "turbo_json_helpers.h"
#include <TurboJSON/parse.h>
#include <string.h>
#include <stdlib.h>

#if (defined _MSC_VER ) || (defined __APPLE__) || (defined __linux__)

static unsigned athena_strnlen(const char *s, unsigned long max, unsigned long i){
    if(!max || s[0]=='\0')
        return i;
    else
        return athena_strnlen(s+1, max-1, i+1);
}
#else
/* I simply do not trust you, Sun Studio, or Open Watcom, or whoever you are! */
static unsigned athena_strnlen(const char *s, unsigned long max, unsigned long i){
athena_strnlen_begin:
    if(!max || s[0]=='\0')
        return i;
    else{
        s++;
        max--;
        i++;
        goto athena_strnlen_begin;
    }
}
#endif


struct Athena_Player *Athena_ConquestCondition(const struct Athena_Field *field, unsigned num_players){
    /* A player is winning if they have any units at all. If we have more than one, we have no winner yet. */
    struct Athena_Player *winning_player = NULL;
    const struct Athena_UnitList *units = field->units;
    if(!units)
        return NULL;

    do{
        /* Skip neutral units. */
        if(units->unit.owner!=NULL){

            /* If we have no winning player */
            if(!winning_player){
                winning_player = units->unit.owner;
            }
            /* If we have a winner, but we see a unit of another team, we have no winner yet. */
            else if(winning_player!=units->unit.owner){
                return NULL;
            }
        }
    }while((units = units->next));

    if(!winning_player)
        return NULL;
    else
        return winning_player;
}

int Athena_Game(struct Athena_Field *field, unsigned num_players, struct Athena_Player *players, struct Athena_Window *window, athena_field_condition win_condition){
    struct Athena_GameState game_state;
    struct Athena_Thread *const server_thread = Athena_CreateThread(Athena_ServerThreadWrapper, &game_state);

    memset(&game_state, 0, sizeof(struct Athena_GameState));

    game_state.monitor = Athena_CreateMonitor();
    game_state.field = field;
    game_state.num_players = num_players;
    game_state.players = players;
    
    game_state.ui.window = window;
    game_state.event.msg = NULL;
    
    Athena_RenewUnitList(game_state.field->units);

    Athena_UIInit(&game_state);

    Athena_CreateImage(&game_state.ui.framebuffer, Athena_WindowWidth(window), Athena_WindowHeight(window));

    Athena_StartThread(server_thread);
athena_game_start:

    Athena_UIThreadFrame(&game_state);

    Athena_MillisecondSleep(10); 

    Athena_LockMonitor(game_state.monitor);
    {
        const int status = game_state.status;
        Athena_UnlockMonitor(game_state.monitor);
        if(status==0)
            goto athena_game_start;
        else{
            Athena_NotifyMonitor(game_state.monitor);
        }
    }

    Athena_JoinThread(server_thread);
    Athena_DestroyThread(server_thread);

    return 0;
}

void Athena_AppendMessageList(struct Athena_MessageList **to, struct Athena_MessageList *next){
    if(to[0]==NULL)
        to[0] = next;
    else
        Athena_AppendMessageList(&(to[0]->next), next);
}

static const char athena_end_turn_message_string[] = "{\n\"type\":\"EndTurn\"\n}";
char *Athena_CreateEndTurnMessage(int *size){
    char * const message_string = malloc(size[0] = (sizeof athena_end_turn_message_string));
    memcpy(message_string, athena_end_turn_message_string, *size);
    size[0]--;
    return message_string;
}

static const char athena_movement_message_string[] =
"{\n    \"type\":\"MoveUnit\",\n    \"from\":{\"x\":%i, \"y\":%i, \"type\":\"%s\"},\n    \"to\":{\"x\":%i, \"y\":%i}\n}\n";
char *Athena_CreateMovementMessage(int *size, struct Athena_Unit *that, int to_x, int to_y){
    const char * const typename = (that->clazz)?that->clazz->name:"";
    const unsigned name_len = strlen(typename);
    char *const message_string = malloc(sizeof(athena_movement_message_string) + 200 + name_len);
    sprintf(message_string, athena_movement_message_string, that->x, that->y, typename, to_x, to_y);

    size[0] = athena_strnlen(message_string, sizeof(athena_movement_message_string) + 200, 0);
    return message_string;
}

static const char athena_attack_message_string[] =
"{\n    \"type\":\"AttackUnit\",\n    \"from\":{\"x\":%i, \"y\":%i, \"type\":\"%s\"},\n    \"to\":{\"x\":%i, \"y\":%i}\n}\n";
char *Athena_CreateAttackMessage(int *size, struct Athena_Unit *that, int to_x, int to_y){
    const char * const typename = (that->clazz)?that->clazz->name:"";
    const unsigned name_len = strlen(typename);
    char *const message_string = malloc(sizeof(athena_attack_message_string) + 200 + name_len);
    sprintf(message_string, athena_attack_message_string, that->x, that->y, typename, to_x, to_y);

    size[0] = athena_strnlen(message_string, sizeof(athena_attack_message_string) + 200, 0);
    return message_string;
}

/* ========================================================================== */
/* Game Tests */
/* ========================================================================== */

int Athena_Test_AthenaStrnlen0Image(){
    static const char * const test_str = "";
    return athena_strnlen(test_str, 0, 0) == 0;
}

int Athena_Test_AthenaStrnlen1Image(){
    static const char * const test_str = "";
    return athena_strnlen(test_str, 99, 0) == 0;
}

int Athena_Test_AthenaStrnlen2Image(){
    static const char * const test_str = "asdf";
    return athena_strnlen(test_str, 99, 0) == 4;
}

int Athena_Test_AthenaStrnlen3Image(){
    static const char * const test_str = "asdf";
    return athena_strnlen(test_str, 3, 0) == 3;
}

int Athena_Test_CreateEndTurnMessage(){
    struct Turbo_Value value;
    int size;
    const char *msg_text = Athena_CreateEndTurnMessage(&size);
    Turbo_Value(&value, msg_text, msg_text + size);
    {
        const struct Turbo_Value
            *const type_obj = Turbo_Helper_GetConstObjectElement(&value, "type");
        
        const int err = Turbo_Helper_CompareStringConstant(type_obj, "EndTurn");
        
        Turbo_FreeParse(&value);
        
        return err;
    }
}

/*
struct Athena_Unit {
    const struct Athena_Class *clazz;
    unsigned owner;
    float health;
    unsigned x, y, movement, actions;
    struct Athena_Animation sprite;
};
*/

#define c_to_x 97
#define c_to_y 86
#define c_from_x 54
#define c_from_y 3

int Athena_Test_CreateMovementMessage(){
    struct Turbo_Value value;

    int size, new_from_x, new_from_y, new_to_x, new_to_y;

    struct Athena_Unit unit = { NULL, 0, 0.0, c_from_x, c_from_y, 0, 0, {0, NULL} };
    
    const char *msg_text = Athena_CreateMovementMessage(&size, &unit, c_to_x, c_to_y);
    Turbo_Value(&value, msg_text, msg_text + size);

    {
        const int err = Athena_GetJSONToAndFrom(&value, &new_from_x, &new_from_y, &new_to_x, &new_to_y);

        Turbo_FreeParse(&value);

        return (err==0) && new_from_x==c_from_x && new_from_y==c_from_y && new_to_x==c_to_x && new_to_y==c_to_y;
    }
}

static struct Athena_Test athena_tests[] = {
    ATHENA_TEST(Athena_Test_AthenaStrnlen0Image),
    ATHENA_TEST(Athena_Test_AthenaStrnlen1Image),
    ATHENA_TEST(Athena_Test_AthenaStrnlen2Image),
    ATHENA_TEST(Athena_Test_AthenaStrnlen3Image),
    ATHENA_TEST(Athena_Test_CreateEndTurnMessage),
    ATHENA_TEST(Athena_Test_CreateMovementMessage)
};

ATHENA_TEST_FUNCTION(Athena_GameTest, athena_tests)
