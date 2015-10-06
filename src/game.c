#include "game.h"
#include "unit.h"

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

int Athena_Game(struct Athena_Field *field, unsigned num_players, athena_field_condition win_condition){
    unsigned winner = 0;
    do{
        
        
        
    }while(!(winner = win_condition(field, num_players)));
    return 0;
}

void Athena_AppendMessageList(struct Athena_MessageList **to, struct Athena_MessageList *next){
    if(to==NULL)
        to[0] = next;
    else
        Athena_AppendMessageList(&(to[0]->next), next);
}
