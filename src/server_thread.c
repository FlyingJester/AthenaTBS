#include "server_thread.h"
#include "turbo_json_helpers.h"
#include <TurboJSON/parse.h>
#include <stdlib.h>
#include <stdint.h>

void Athena_ServerThreadWrapper(void *that){
    Athena_ServerThread(that);
}

#define ATHENA_MESSAGE_TYPE(TYPE_2) Athena_ ## TYPE_2 ## MessageType

#define ATHENA_MESSAGE_TYPE_STRING(TYPE_1)\
const char ATHENA_MESSAGE_TYPE(TYPE_1) [] = #TYPE_1

ATHENA_MESSAGE_TYPE_STRING(EndTurn);
ATHENA_MESSAGE_TYPE_STRING(MoveUnit);
ATHENA_MESSAGE_TYPE_STRING(AttackUnit);
ATHENA_MESSAGE_TYPE_STRING(BuildUnit);
ATHENA_MESSAGE_TYPE_STRING(BuildTile);

#undef ATHENA_MESSAGE_TYPE_STRING

static int athena_handle_message_iter(struct Athena_MessageList *msg, struct Athena_GameState *that){
    if(!msg){
        return 0;
    }
    else{
        const struct Turbo_Value *type = Turbo_Helper_GetConstObjectElement(&msg->value, "type");
        if(type){
            if(Turbo_Helper_CompareStringConstant(type, ATHENA_MESSAGE_TYPE(EndTurn))){
                that->whose_turn = (that->whose_turn+1) % that->num_players;
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
