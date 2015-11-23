#include "window/window.h"
#include "game.h"
#include "titlescreen.h"

int main(int argc, char *argv[]){
    struct Athena_Options option;
    const int err1 = Athena_LoadOptions(athena_default_settings_path, &option);
    struct Athena_Window * const window = Athena_CreateWindow(option.screen_w, option.screen_h, "Athena Hotseat Test");
    struct Athena_Image temporary_framebuffer;
    enum Athena_GameType game_type;

    Athena_CreateImage(&temporary_framebuffer, option.screen_w, option.screen_h);
    Athena_ShowWindow(window);
    game_type = Athena_Titlescreen(window, &temporary_framebuffer);
    return game_type || err1;
}
