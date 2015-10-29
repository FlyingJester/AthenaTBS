#include "game.h"
#include "unit.h"
#include "ui_thread.h"
#include "server_thread.h"
#include "thread/thread.h"
#include "time/sleep.h"
#include <string.h>
#include <stdlib.h>

#if (defined _MSC_VER ) || (defined __APPLE__) || (defined __linux__)

static unsigned athena_strnlen(const char *s, unsigned long max, unsigned long i){
    if(!max || s[0]=='\0')
        return i;
    else
        return athena_strnlen(s, max-1, i+1);
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


unsigned Athena_ConquestCondition(const struct Athena_Field *field, unsigned num_players){
    /* A player is winning if they have any units at all. If we have more than one, we have no winner yet. */
    unsigned winning_player = 0;
    const struct Athena_UnitList *units = field->units;

    /* Somehow we only no units. End the game. */
    if(!units)
        return 1;

    do{
        /* Skip neutral units. */
        if(units->unit.owner!=0){
        
            /* If we have no winning player */
            if(!winning_player){
                winning_player = units->unit.owner;
            }
            /* If we have a winner, but we see a unit of another team, we have no winner yet. */
            else if(winning_player!=units->unit.owner){
                return 0;
            }
        }
    }while((units = units->next));
    
    /* Somehow we only have neutral units. End the game. */
    if(!winning_player)
        return 1;
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
"{\n    \"type\":\"MoveUnit\",\n    \"from\":{\"x\":%i, \"y\":%i},\n    \"to\":{\"x\":%i, \"y\":%i}\n}\n";
char *Athena_CreateMovementMessage(int *size, struct Athena_Unit *that, int to_x, int to_y){
    /* No number entered will be more than 0xFFFF, or 65,000ish. */

    char *const message_string = malloc(sizeof(athena_movement_message_string) + 200);
    sprintf(message_string, athena_movement_message_string, that->x, that->y, to_x, to_y);
    
    fputs("Creating message string: ", stderr);
    fputs(message_string, stderr);
    fputc('\n', stderr);
    
    size[0] = athena_strnlen(message_string, sizeof(athena_movement_message_string) + 200, 0);
    return message_string;
}
