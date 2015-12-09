#include "server_thread.h"
#include "player.h"
#include "unit.h"
#include "turbo_json_helpers.h"
#include <TurboJSON/parse.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

void Athena_ServerThreadWrapper(void *that){
    Athena_ServerThread(that);
}

const char *Athena_ServerMessageTypeString(enum Athena_ServerMessageTypes e){
    switch(e){
        case EndTurn:
            return "EndTurn";
        case MoveUnit:
            return "MoveUnit";
        case AttackUnit:
            return "AttackUnit";
        case BuildUnit:
            return "BuildUnit";
        case BuildTile:
            return "BuildTile";
        case NumServerMessageTypes:
            return "NumServerMessageTypes";
        case Nothing:
        default:
            return "Nothing";
    }
}

enum Athena_ServerMessageTypes Athena_ServerMessageTypeEnum(const char *str){
    return Athena_ServerMessageTypeEnumN(str, strlen(str));
}

enum Athena_ServerMessageTypes Athena_ServerMessageTypeEnumN(const char *str, unsigned len){
    int i = 0;
    while(i<NumServerMessageTypes){
        const char * const that = Athena_ServerMessageTypeString(i);
        if(strlen(that) == len && memcmp(that, str, len) == 0)
            break;
        else
            i++;
    }
    return i;
}

struct Athena_InternalServerMessage{
    enum Athena_ServerMessageTypes type;
    
    
};

static int athena_get_json_to_and_from_inner(const struct Turbo_Value *to, const struct Turbo_Value *from, 
    int *from_x_out, int *from_y_out, int *to_x_out, int *to_y_out){

    if(from && to){
        const struct Turbo_Value
            *const from_x= Turbo_Helper_GetConstObjectElement(from, "x"),
            *const from_y= Turbo_Helper_GetConstObjectElement(from, "y"),
            *const to_x  = Turbo_Helper_GetConstObjectElement(to, "x"),
            *const to_y  = Turbo_Helper_GetConstObjectElement(to, "y");
        if(from_x && from_y && to_x && to_y &&
            from_x->type == TJ_Number &&
            from_y->type == TJ_Number &&
            to_x->type == TJ_Number &&
            to_y->type == TJ_Number){
            
            from_x_out[0] = from_x->value.number;
            from_y_out[0] = from_y->value.number;
            to_x_out[0] = to_x->value.number;
            to_y_out[0] = to_y->value.number;
            
            return 0;
        }
    }
    return 1;
}

int Athena_GetJSONToAndFrom(const struct Turbo_Value *obj, int *from_x_out, int *from_y_out, int *to_x_out, int *to_y_out){
    const struct Turbo_Value
        *const from = Turbo_Helper_GetConstObjectElement(obj, "from"),
        *const to = Turbo_Helper_GetConstObjectElement(obj, "to");

    return athena_get_json_to_and_from_inner(to, from, from_x_out, from_y_out, to_x_out, to_y_out);
}

int Athena_GetJSONToAndFromWithType(const struct Turbo_Value *obj, const char **name, unsigned *name_len, int *from_x_out, int *from_y_out, int *to_x_out, int *to_y_out){
    const struct Turbo_Value
        *const from = Turbo_Helper_GetConstObjectElement(obj, "from"),
        *const to = Turbo_Helper_GetConstObjectElement(obj, "to");

    if(from){
        const struct Turbo_Value
            *const from_type = Turbo_Helper_GetConstObjectElement(from, "type");
        
        if(!from_type->type==TJ_String)
            return 1;
        
        name[0] = from_type->value.string;
        name_len[0] = from_type->length;
            
        return athena_get_json_to_and_from_inner(to, from, from_x_out, from_y_out, to_x_out, to_y_out);
    }
    else
        return 1;
}

static void athena_clear_corpses_iter(struct Athena_UnitList *first, struct Athena_UnitList *second,
    void(*death_callback)(struct Athena_Unit *dying)){
    
    assert(first);
    assert(first->next == second);
    
    if(!second)
        return;
    else{
        if(second->unit.health==0){
            first->next = second->next;
            if(death_callback)
                death_callback(&second->unit);
            free(second);
        }
        athena_clear_corpses_iter(first->next, second->next, death_callback);
    }
}

static void athena_clear_corpses(struct Athena_UnitList **list, void(*death_callback)(struct Athena_Unit *dying)){
    if(!list[0])
        return;
    else if(list[0]->next==NULL){
        if(!list[0]->unit.health){
            if(death_callback)
                death_callback(&list[0]->unit);
            free(list[0]);
            list[0] = NULL;
        }
    }
    else{
        athena_clear_corpses_iter(list[0], list[0]->next, death_callback);
    }
}

static int athena_handle_message_iter(struct Athena_MessageList *msg, struct Athena_GameState *that){
    if(!msg){
        return 0;
    }
    else{
        const struct Turbo_Value *type = Turbo_Helper_GetConstObjectElement(&msg->value, "type");
        if(type && 
            type->type == TJ_String){

            const enum Athena_ServerMessageTypes msg_type = Athena_ServerMessageTypeEnumN(type->value.string, type->length);
            switch(msg_type){
                case EndTurn:
                    Athena_RenewUnitList(that->field->units);
                    that->whose_turn = (that->whose_turn+1) % that->num_players;
                    Athena_GivePlayerIncome(&that->players[that->whose_turn], that->field->units);
                    athena_clear_corpses(&that->field->units, NULL);
                    break;
                case MoveUnit:
                    {
                        int from_x, from_y, to_x, to_y;
                        const char *type;
                        unsigned type_len;
                        struct Athena_Unit *movable;
                        if(Athena_GetJSONToAndFromWithType(&msg->value, &type, &type_len, &from_x, &from_y, &to_x, &to_y)!=0)
                            break;
                        if((movable = Athena_FindUnitTypeAtN(that->field->units, type, type_len, from_x, from_y))){
                            movable->x = to_x;
                            movable->y = to_y;
                            movable->movement = 0;
                        }
                        else{
                            fprintf(stderr, "[athena_handle_message_iter]Attempt to move non-existant unit at %i, %i to %i, %i\n", from_x, from_y, to_x, to_y);
                        }
                    }
                    break;
                case AttackUnit:
                    {
                        int from_x, from_y, to_x, to_y;
                        const char *type;
                        unsigned type_len;
                        struct Athena_Unit *attacker, *attackee;
                        if(Athena_GetJSONToAndFromWithType(&msg->value, &type, &type_len, &from_x, &from_y, &to_x, &to_y)!=0)
                            break;
                        if((attacker = Athena_FindUnitTypeAtN(that->field->units, type, type_len, from_x, from_y)) && (attacker->actions) && 
                            (attackee = Athena_FindAttackableUnitAt(that->field->units, to_x, to_y))){
                            attacker->actions = 0;
                            Athena_Attack(attacker, attackee);
                        }
                        else{
                            if(!attacker)
                                fprintf(stderr, "[athena_handle_message_iter]Non-existant attacking unit of type %s at %i, %i\n", type, from_x, from_y);
                            else if(!attacker->actions)
                                fprintf(stderr, "[athena_handle_message_iter]Attacking unit at %i, %i of type %s has no more actions available.\n", from_x, from_y, attacker->clazz->name);
                            else if(!attackee){
                                fprintf(stderr, "[athena_handle_message_iter]Non-existant attack target at %i, %i\n", to_x, to_y);
                            }
                        }
                        athena_clear_corpses(&that->field->units, NULL);
                    }
                    break;
                default:
                    fprintf(stderr, "[athena_handle_message_iter]Dropping message of type \"%s\" (%i)\n", Athena_ServerMessageTypeString(msg_type), msg_type);
            }
            
        }
        return athena_handle_message_iter(msg->next, that);
    }
}

static void athena_free_messages(struct Athena_MessageList *msg){
    if(msg){
        struct Athena_MessageList * const next = msg->next;
        Turbo_FreeParse(&msg->value);
        free(msg->msg_text);
        msg->msg_text = NULL;
        free(msg);
        msg = NULL;
        athena_free_messages(next);
    }
}

int Athena_ServerThread(struct Athena_GameState *that){
    struct Athena_MessageList *msg;
    Athena_LockMonitor(that->monitor);
server_thread_begin:

    athena_handle_message_iter(that->event.msg, that);

    msg = that->event.msg;
    that->event.msg = NULL;

    Athena_UnlockMonitor(that->monitor);

    if(msg)
        athena_free_messages(msg);

    Athena_LockMonitor(that->monitor);
    if(that->status==0){
        Athena_WaitMonitor(that->monitor);
        goto server_thread_begin;    
    }

    Athena_UnlockMonitor(that->monitor);

    return 1;
}
