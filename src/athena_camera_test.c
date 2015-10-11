#include "window/window.h"
#include "game.h"
#include "player.h"

const unsigned scr_width = 400, scr_height = 300;

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

int main(int argc, char *argv[]){
    struct Athena_Window * const window = Athena_CreateWindow(scr_width, scr_height, "Athena Test");
    struct Athena_Player players[] = {{0, 0, 0, "Flying Jester", {NULL, 0, 0}, 0xFF0000FF}};
    struct Athena_Field field;
    
    Athena_ShowWindow(window);
    {
        const int err = Athena_LoadFieldFromFile("res/maps/cam_test.json", &field);
/*
        const int err = Athena_LoadFieldFromMemory(field_src, sizeof(field_src), &field, "");
*/
        if(err!=0){
            fprintf(stderr, "[athena_camera_test][main]Could not load field. Error code %i\n", err);
            return 1;
        }
    }
    
    Athena_Game(&field, sizeof(players) / sizeof(players[0]),  players, window, Athena_ConquestCondition);

    return 0;
}
