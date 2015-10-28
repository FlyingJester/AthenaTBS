#include "server_thread.h"
#include "unit.h"
#include "turbo_json_helpers.h"
#include <TurboJSON/parse.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

static int athena_get_to_and_from(const struct Turbo_Value *obj, int *from_x_out, int *from_y_out, int *to_x_out, int *to_y_out){
    const struct Turbo_Value
        *const from = Turbo_Helper_GetConstObjectElement(obj, "from"),
        *const to = Turbo_Helper_GetConstObjectElement(obj, "to");
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
                    that->whose_turn = (that->whose_turn+1) % that->num_players;
                    break;
                case MoveUnit:
                    {
                        int from_x, from_y, to_x, to_y;
                        struct Athena_Unit *movable;
                        if(athena_get_to_and_from(&msg->value, &from_x, &from_y, &to_x, &to_y)!=0)
                            break;
                        if((movable = Athena_FindUnitAt(that->field->units, from_x, from_y))){
                            movable->x = to_x;
                            movable->y = to_y;
                        }
                        else{
                            fprintf(stderr, "[athena_handle_message_iter]Attempt to move non-existant unit at %i, %i to %i, %i\n", from_x, from_y, to_x, to_y);
                        }
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
