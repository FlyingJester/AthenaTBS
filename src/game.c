#include "game.h"
#include "unit.h"
#include "ui_thread.h"
#include "server_thread.h"
#include "thread/thread.h"
#include "time/sleep.h"
#include <string.h>

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

    Athena_StartThread(server_thread);
athena_game_start:

    Athena_UIThreadFrame(&game_state);

    Athena_MillisecondSleep(1);

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
    if(to==NULL)
        to[0] = next;
    else
        Athena_AppendMessageList(&(to[0]->next), next);
}
