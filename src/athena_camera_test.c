#include "window/window.h"
#include "game.h"
#include "player.h"

const unsigned scr_width = 720, scr_height = 450;

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
    struct Athena_Window * const window = Athena_CreateWindow(scr_width, scr_height, "Athena Test");
    struct Athena_Player players[] = {{0, 0, 0, "Flying Jester", {NULL, 0, 0}, 0xFF0000FF}, {0, 0, 0, "Link", {NULL, 0, 0}, 0xFF0FF0F0}};
    struct Athena_Field field;
    
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

    Athena_Game(&field, sizeof(players) / sizeof(players[0]),  players, window, Athena_ConquestCondition);

    return 0;
}
