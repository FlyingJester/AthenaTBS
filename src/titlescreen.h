#pragma once
#include "game.h"

enum Athena_GameType {
    quit_game_mode,
    hotseat_multi,
    network_multi,
    entire_map,
    scenario,
    ATHENA_NUM_GAMETYPES
};

#ifdef __cplusplus
extern "C"
#endif
enum Athena_GameType Athena_Titlescreen(struct Athena_Window *window, struct Athena_Image *framebuffer);
