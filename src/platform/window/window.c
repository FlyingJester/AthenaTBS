#include "window.h"
#include "private_window.h"
#include <stdlib.h>

struct Athena_Window {
    void *handle;
    unsigned w, h;
    unsigned char is_shown;
    unsigned char dirty;
};

struct Athena_Window *Athena_CreateWindow(unsigned w, unsigned h, const char *title){
    struct Athena_Window *window = malloc(sizeof(struct Athena_Window));
    window->handle = Athena_Private_CreateHandle();
    if(!window)
        return NULL;
    if(Athena_Private_CreateWindow(window->handle, 200, 200, w, h, title)<0)
        return NULL;
    
    window->w = w;
    window->h = h;
  
    return window;
}

int Athena_DestroyWindow(struct Athena_Window *that){
    void * const handle = (that)?(that->handle):NULL;

    if(!handle)
        return -1;

    free(that);
    return Athena_Private_DestroyHandle(handle);

}

int Athena_ShowWindow(struct Athena_Window *that){
    return Athena_Private_ShowWindow(that->handle);
}

int Athena_HideWindow(struct Athena_Window *that){
    return Athena_Private_HideWindow(that->handle);
}

int Athena_IsWindowShown(struct Athena_Window *that){
    return 0;
}

int Athena_WindowWidth(struct Athena_Window *that){
    if(!that)
        return -1;
    return that->w;
}

int Athena_WindowHeight(struct Athena_Window *that){
    if(!that)
        return -1;
    return that->h;
}

int Athena_Update(struct Athena_Window *that, enum Athena_ImageFormat format, const void *RGB){
    that->dirty = 1;
    return Athena_Private_Update(that->handle, format, RGB, that->w, that->h);
}

int Athena_FlipWindow(struct Athena_Window *that){
    if(!that->dirty)
        return 0;
    that->dirty = 0;

    return Athena_Private_FlipWindow(that->handle);
}

unsigned Athena_GetEvent(struct Athena_Window *that, struct Athena_Event *to){
    return Athena_Private_GetEvent(that, to);
}

int Athena_IsKeyPressed(struct Athena_Window *that, unsigned key){
    return Athena_Private_IsKeyPressed(that->handle, key);
}

int Athena_GetMousePosition(struct Athena_Window *that, int *x, int *y){
    return Athena_Private_GetMousePosition(that->handle, x, y);
}

unsigned Athena_GetMouseX(struct Athena_Window *that){
    int x, y;
    Athena_Private_GetMousePosition(that->handle, &x, &y);
    if(x<0)
        return 0;
    else
        return x;
}

unsigned Athena_GetMouseY(struct Athena_Window *that){
    int x, y;
    Athena_Private_GetMousePosition(that->handle, &x, &y);
    
    if(y<0)
        return 0;
    else
        return y;
}
