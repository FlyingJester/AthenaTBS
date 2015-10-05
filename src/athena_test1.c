#include "field.h"
#include "window/window.h"
#include "image.h"

int main(int argc, char *argv[]){
    struct Athena_Event event;
    struct Athena_Image image[2];
    const char *image_name[2] = {"test.png", "test.png"};
    struct Athena_Window * const window = Athena_CreateWindow(800, 600, "Athena Test");

    if(!window || Athena_ShowWindow(window)<0)
        return 1;

    event.type = athena_unknown_event;
    
    if(argc>1)
        image_name[0] = argv[1];
    if(argc>2)
        image_name[1] = argv[2];

    if(Athena_LoadPNG(&image[0], image_name[0])!=ATHENA_LOADPNG_SUCCESS){
        puts("Error loading image file 1");
        return 1;
    }
    if(Athena_LoadPNG(&image[1], image_name[1])!=ATHENA_LOADPNG_SUCCESS){
        puts("Error loading image file 2");
        return 1;
    }

    Athena_Blit(&image[1], &image[0], image[0].w>>1, image[0].h>>1);

    do{
        if(Athena_GetEvent(window, &event)){
            Athena_DrawImage(window, 16, 16, image[0].w, image[0].h, 0, image[0].pixels);
            Athena_FlipWindow(window);
        }
    }while(event.type != athena_quit_event);

    return argc;
}
