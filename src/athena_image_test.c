#include "field.h"
#include "window/window.h"
#include "image.h"

const unsigned scr_width = 400, scr_height = 300;

int main(int argc, char *argv[]){
    struct Athena_Event event;
    struct Athena_Image frame_buffer;
    struct Athena_Window * const window = Athena_CreateWindow(scr_width, scr_height, "Athena Image Test");

    if(!frame_buffer.pixels || !window || Athena_ShowWindow(window)<0)
        return 1;

    event.type = athena_unknown_event;
    if(argc<2 || Athena_LoadAuto(&frame_buffer, argv[1])!=ATHENA_LOADPNG_SUCCESS)
        return 0;

    do{
        if(Athena_GetEvent(window, &event)){
            Athena_DrawImage(window, 0, 0, frame_buffer.w, frame_buffer.h, 0, frame_buffer.pixels);
            Athena_FlipWindow(window);

        }
    }while(event.type != athena_quit_event);
    return 0;
}
