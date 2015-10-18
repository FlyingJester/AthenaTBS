#include "server_thread.h"
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
                default:
                    fprintf(stderr, "[athena_handle_message_iter]Dropping message of type \"%s\" (%i)\n", Athena_ServerMessageTypeString(msg_type), msg_type);
            }
            
        }
        return athena_handle_message_iter(msg->next, that);
    }
}

static void athena_free_message_json(struct Athena_MessageList *msg){
    if(msg){
        Turbo_FreeParse(&msg->value);
        athena_free_message_json(msg->next);
    }
}

static void athena_free_message_array(struct Athena_MessageList *msg){
    if(msg){
        struct Athena_MessageList * const next = msg->next;
        free(msg->msg_text);
        Turbo_FreeParse(&msg->value);
        free(msg);
        athena_free_message_array(next);
    }
}

static void athena_free_messages(struct Athena_MessageList *msg){
    athena_free_message_json(msg);
    athena_free_message_array(msg);
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
