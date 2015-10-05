#include "window.h"
#include "private_window.h"
#include <X11/Xlib.h>
#include <stdlib.h>
#include <assert.h>

static Display *display = NULL;

#define ATHENA_VERIFY(A_HANDLE)\
    A_HANDLE;\
    if(!display)\
        return -1;\
    if(!A_HANDLE)\
        return -1

struct Athena_X_Window {
    Window root_window, window;
    Colormap color_map;
    int screen;
    GC gc;
};

static unsigned athena_private_set_color(const struct Athena_Color *color, struct Athena_X_Window *x_window, XColor *x_color){
    Athena_Common_ColorToUnsignedShort(color, &x_color->red, &x_color->green, &x_color->blue, NULL);
    
    if(!(x_color->red & 0xFFF0) && !(x_color->green & 0xFFF0) && !(x_color->blue & 0xFFF0)){
        XSetForeground(display, x_window->gc, BlackPixel(display, x_window->screen));
    }
    else if((x_color->red & 0xFFF0) == 0xFFF0 && (x_color->green & 0xFFF0) == 0xFFF0 && (x_color->blue & 0xFFF0) == 0xFFF0){
        XSetForeground(display, x_window->gc, WhitePixel(display, x_window->screen));
    }
    else{
        x_color->pixel = 0;
        x_color->flags = DoRed|DoGreen|DoBlue;
        
        XAllocColor(display, x_window->color_map, x_color);

        XSetForeground(display, x_window->gc, x_color->pixel);
        
        return 1;
    }
    
    return 0;
}

int athena_private_unset_color(unsigned must_dealloc, const struct Athena_Color *color, struct Athena_X_Window *x_window, XColor *x_color){
    if(must_dealloc)
        XFreeColors(display, x_window->color_map, &x_color->pixel, 1, 0);
    return 0;
}

void *Athena_Private_CreateHandle(){
    struct Athena_X_Window *x_window;

    if(!display && !(display = XOpenDisplay(NULL)))
        return NULL;

    assert(display);

    x_window = malloc(sizeof(struct Athena_X_Window));
    x_window->screen = DefaultScreen(display);
    x_window->root_window = RootWindow(display, x_window->screen);
    x_window->color_map = DefaultColormap(display, x_window->screen);
    
    return x_window;
}

int Athena_Private_CreateWindow(void *handle, int x, int y, unsigned w, unsigned h, const char *title){
    XEvent x_event;
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    
    {
        const int black = BlackPixel(display, x_window->screen);
        x_window->window = XCreateSimpleWindow(display, x_window->screen, x, y, w, h, 0, black, black); 
    }

    XSelectInput(display, x_window->window, StructureNotifyMask);
    x_window->gc = XCreateGC(display, x_window->window, 0, NULL);

    if(title)
        XStoreName(display, x_window->window, title);
    
    do{
        XNextEvent(display, &x_event);
    }while(x_event.type != MapNotify);
    
    return 0;
}

int Athena_Private_DestroyHandle(void *handle){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    XFreeGC(display, x_window->gc);
    XDestroyWindow(display, x_window->window);
    return 0;
}

int Athena_Private_ShowWindow(void *handle){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);

    XMapWindow(display, x_window->window);
    return 0;
}
int Athena_Private_HideWindow(void *handle){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    
    XUnmapWindow(display, x_window->window);
    return 0;
}

int Athena_Private_DrawImage(void *handle, int x, int y, unsigned w, unsigned h, unsigned format, const void *RGB);

int Athena_Private_DrawRect(void *handle, int x, int y, unsigned w, unsigned h, const struct Athena_Color *color){
    XColor x_color;
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    
    {
        const unsigned must_dealloc = athena_private_set_color(color, x_window, &x_color);
        XDrawLine(display, x_window->window, x_window->gc, x, y, w, h);
        athena_private_unset_color(must_dealloc, color, x_window, &x_color);
    }

    return 0;
}

int Athena_Private_DrawLine(void *that, int x1, int y1, int x2, int y2, const struct Athena_Color *color);

int Athena_Private_FlipWindow(void *handle){
    XFlush(display);
    return 0;
}

unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to);
