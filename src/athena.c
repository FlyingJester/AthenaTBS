#include "field.h"
#include "window/window.h"
#include "image.h"

const unsigned scr_width = 800, scr_height = 600;

int main(int argc, char *argv[]){
    struct Athena_Event event;
    struct Athena_Image frame_buffer;
    struct Athena_Window * const window = Athena_CreateWindow(scr_width, scr_height, "Athena Test");
    Athena_CreateImage(&frame_buffer, scr_width, scr_height);

    if(!frame_buffer.pixels || !window || Athena_ShowWindow(window)<0)
        return 1;

    event.type = athena_unknown_event;

    do{

        if(Athena_GetEvent(window, &event)){
            Athena_DrawImage(window, 0, 0, frame_buffer.w, frame_buffer.h, 0, frame_buffer.pixels);
            Athena_FlipWindow(window);

        }

    }while(event.type != athena_quit_event);

    return argc;
}
