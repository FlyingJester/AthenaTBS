#include "server_thread.h"
#include "turbo_json_helpers.h"
#include <TurboJSON/parse.h>
#include <stdlib.h>
#include <stdint.h>

void Athena_ServerThreadWrapper(void *that){
    Athena_ServerThread(that);
}

static int athena_handle_message_iter(struct Athena_MessageList *msg){
    if(!msg){
        return 0;
    }
    else{
        const struct Turbo_Value *type = Turbo_Helper_GetConstObjectElement(&msg->value, "type");
        if(type){
        
        }
        return athena_handle_message_iter(msg->next);
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
    Athena_LockMonitor(that->event.monitor);
server_thread_begin:

    athena_handle_message_iter(that->event.msg);

    msg = that->event.msg;
    that->event.msg = NULL;

    Athena_UnlockMonitor(that->event.monitor);

    if(msg)
        athena_free_messages(msg);

    Athena_LockMonitor(that->event.monitor);
    if(that->event.status==0){
        Athena_WaitMonitor(that->event.monitor);
        goto server_thread_begin;    
    }

    Athena_UnlockMonitor(that->event.monitor);

    return 1;
}
