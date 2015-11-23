#include "window/window.h"
#include "game.h"
#include "player.h"
#include "unit_classes.h"
#include "load_opus.h"
#include "audio/audio.h"
#include <string.h>

const char field_src[] = "{\
    \"tileset\":\"res/tilesets/field1/ts.json\",\
    \"attributes\":{\"width\":8, \"height\":8},\
    \"field\":[\
        [0, 0, 1, 0, 0, 0, 0, 0],\
        [0, 0, 1, 0, 1, 1, 0, 0],\
        [0, 1, 1, 0, 1, 1, 1, 0],\
        [0, 1, 0, 0, 0, 0, 0, 0],\
        [0, 1, 1, 0, 0, 0, 0, 0],\
        [0, 0, 1, 1, 0, 0, 1, 1],\
        [0, 0, 0, 1, 1, 1, 1, 0],\
        [0, 0, 0, 0, 0, 0, 0, 0]\
    ]\
}\
";

static const char * const flag_1 = "res/images/jest.png", * const flag_2 = "res/images/legend.png";

int main(int argc, char *argv[]){
    struct Athena_Options option;
    const int err1 = Athena_LoadOptions("athena_settings.json", &option);
    struct Athena_Window * const window = Athena_CreateWindow(option.screen_w, option.screen_h, "Athena Test");
    struct Athena_Player players[] = {{0, 0, 0, "Flying Jester", {NULL, 0, 0}, 0xFF0000FF, 1}, {0, 0, 0, "Link", {NULL, 0, 0}, 0xFF0FF0F0, 1}};
    struct Athena_Field field;

    struct Athena_Sound *sound = Athena_LoadOpusFile("res/sounds/night_at_the_river.opus");
    struct Athena_SoundConfig config;
    Athena_SoundGetConfig(sound, &config);
    config.loop = 1;
    
    Athena_SoundSetConfig(sound, &config);
    Athena_SoundPlay(sound);

    if(err1){}

    memset(&field, 0, sizeof(struct Athena_Field));

    Athena_UnitClassesInit();
    
    Athena_ShowWindow(window);
    {
        const int err = Athena_LoadFieldFromFile("res/maps/cam_test2.json", &field);
/*
        const int err = Athena_LoadFieldFromMemory(field_src, sizeof(field_src), &field, "");
*/
        if(err!=0){
            fprintf(stderr, "[athena_camera_test][main]Could not load field. Error code %i\n", err);
            return 1;
        }
    }
    
    {
        int err[2];
        err[0] = Athena_LoadAuto(&players[0].flag, flag_1);
        err[1] = Athena_LoadAuto(&players[1].flag, flag_2);
        if(err[0]!=ATHENA_LOADPNG_SUCCESS){
            fprintf(stderr, "[athena_camera_test][main]Could not load flag %s. Error code %i\n", flag_1, err[0]);
            players[0].flag.pixels = NULL;
        }
        if(err[1]!=ATHENA_LOADPNG_SUCCESS){
            fprintf(stderr, "[athena_camera_test][main]Could not load flag %s. Error code %i\n", flag_1, err[1]);
            players[1].flag.pixels = NULL;
        }
    }
    
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Path"),          players + 0, 3, 7);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Path"),          players + 0, 3, 6);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Path"),          players + 0, 3, 5);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Industrial"),    players + 0, 4, 7);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Fortification"), players + 0, 4, 6);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Industrial"),    players + 0, 2, 6);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Coalition Builder"), players + 0, 5, 7);

    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Black Coat"), players + 0, 4, 6);
    
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Path"),          players + 1, 12, 4);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Path"),          players + 1, 13, 4);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Industrial"),    players + 1, 13, 3);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Path"),          players + 1, 14, 4);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Fortification"), players + 1, 14, 3);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Path"),          players + 1, 15, 4);
    Athena_SpawnUnit(&field.units, Athena_BuiltinClass("Coalition Builder"), players + 1, 8, 3);
    
    Athena_Game(&field, sizeof(players) / sizeof(players[0]), players, window, &option, Athena_ConquestCondition);

    return 0;
}
