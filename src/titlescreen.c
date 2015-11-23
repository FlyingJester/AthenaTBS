#include "titlescreen.h"
#include "time/sleep.h"
#include <assert.h>
/*
enum Athena_GameType {
    quit_game_mode
    hotseat_multi,
    network_multi,
    entire_map,
    scenario
};

int Athena_ProcessButtons(struct Athena_GameState *that, struct Athena_ButtonList *buttons, 
    const struct Athena_Event *event, struct Athena_MessageList *messages);
*/

#define BUTTON_WIDTH 128
#define BUTTON_HEIGHT 20

void athena_quit_button_callback(struct Athena_ButtonArgList *args, struct Athena_MessageList *messages){
    int *status = Athena_FindTypeInArgList(args, "screen_state");
    assert(status);
    status[0] = quit_game_mode;
}

enum Athena_GameType athena_titlescreen_iter(struct Athena_Window *window, struct Athena_Image *framebuffer, struct Athena_ButtonList *buttons, int *status){
    if(*status!=-1)
        return *status;
    else{
        struct Athena_Event event;
        struct Athena_Viewport port = {NULL, 0, 0, 0, 0};
        
        port.image = framebuffer; port.w = framebuffer->w; port.h = framebuffer->h;
        Athena_UIDrawButtons(buttons, &port);
        Athena_Update(window, 0, framebuffer->pixels);
        Athena_FlipWindow(window);

        if(!Athena_GetEvent(window, &event)){
            Athena_MillisecondSleep(1);
            return athena_titlescreen_iter(window, framebuffer, buttons, status);
        }
        else{
            if(event.type==athena_quit_event)
                return quit_game_mode;

            Athena_ProcessButtons(NULL, buttons, &event, NULL);

            return athena_titlescreen_iter(window, framebuffer, buttons, status);
        }
    }
}

enum Athena_GameType Athena_Titlescreen(struct Athena_Window *window, struct Athena_Image *framebuffer){

    struct Athena_ButtonList *buttons = NULL;

    struct Athena_ButtonArgList *const arglist = Athena_DefaultButtonArgList(NULL);
    int status = -1;

    struct Athena_Button hotseat_multi_button = {
        - (BUTTON_WIDTH + BUTTON_HEIGHT), BUTTON_HEIGHT * 1,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        "Play Hotseat",
        NULL, NULL, 0
    };
    struct Athena_Button quit_button = {
        - (BUTTON_WIDTH + BUTTON_HEIGHT), BUTTON_HEIGHT * 2,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        "Quit Athena",
        NULL, athena_quit_button_callback, 0
    };

    quit_button.x += framebuffer->w;
    quit_button.arg = arglist;

    hotseat_multi_button.x += framebuffer->w;
    hotseat_multi_button.arg = arglist;

    Athena_AppendButton(&buttons, quit_button);
    Athena_AppendButton(&buttons, hotseat_multi_button);

    Athena_AppendButtonArgList(arglist, &status, "screen_state");
    
    athena_titlescreen_iter(window, framebuffer, buttons, &status);
    
    Athena_FreeButtonArgList(arglist);
    
    return hotseat_multi;
}
