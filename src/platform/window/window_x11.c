#include "window.h"
#include "private_window.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
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
    int mouse_x, mouse_y;
    XVisualInfo *visinfo;
    int framebuffer_info;
    XImage *framebuffer;
    XShmSegmentInfo shminfo;
    GC gc;
};

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

#define PRINT_ALL_X_MODES

#ifdef PRINT_ALL_X_MODES

static const char *class_name(int c){
#define CASE(X) if(c==X) return #X
    CASE(TrueColor);
    CASE(DirectColor);
    CASE(GrayScale);
    CASE(StaticColor);
    CASE(StaticGray);
    return "unknown";
}

#endif

int Athena_Private_CreateWindow(void *handle, int x, int y, unsigned w, unsigned h, const char *title){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    
    {
        const int black = BlackPixel(display, x_window->screen);
        int n = 0, i = 0;
        XVisualInfo template;

        template.screen = x_window->screen;
        x_window->visinfo = XGetVisualInfo(display, VisualScreenMask, &template, &n);
        x_window->framebuffer_info = -1;
        while(i<n){
#ifdef PRINT_ALL_X_MODES
            printf("[Athena_Private_CreateWindow]Visual %i:\n\tVisual 0x%lx\n\tClass %s\n\tDepth %d\n\n",
                i, x_window->visinfo[i].visualid, class_name(x_window->visinfo[i].class), x_window->visinfo[i].depth);
#endif
            if(x_window->visinfo[i].class == TrueColor && x_window->visinfo[i].depth==32){
                x_window->framebuffer_info = i;
                break;
            }
            i++;
        }
        
        if(x_window->framebuffer_info==-1){
            i = 0;
            while(i<n){
                if(x_window->visinfo[i].class == TrueColor && x_window->visinfo[i].depth==24){
                    x_window->framebuffer_info = i;
                    break;
                }
                i++;
            }
        }
        
        if(x_window->framebuffer_info==-1)
            x_window->framebuffer_info = 0;
/*        
        x_window->framebuffer = XShmCreateImage(display, DirectColor, 32, XYBitmap, NULL, &x_window->shminfo, w, h);
  *//*
        x_window->window = XCreateWindow(display, x_window->screen, int x, int y, unsigned int width, unsigned int
              height, unsigned int border_width, int depth, unsigned int class, Visual *visual, unsigned long val-
              uemask, XSetWindowAttributes *attributes);
              */
        x_window->window = XCreateSimpleWindow(display, x_window->root_window, x, y, w, h, 0, black, black);
    }

    XSelectInput(display, x_window->window, StructureNotifyMask);
    x_window->gc = XCreateGC(display, x_window->window, 0, NULL);

    if(title)
        XStoreName(display, x_window->window, title);

    return 0;
}

int Athena_Private_Update(void *handle, unsigned format, const void *RGB, unsigned w, unsigned h){
    return 0;
}

int Athena_Private_DestroyHandle(void *handle){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    XFreeGC(display, x_window->gc);
    XDestroyWindow(display, x_window->window);
    return 0;
}

int Athena_Private_ShowWindow(void *handle){
    XEvent x_event;
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);

    XMapWindow(display, x_window->window);

    do{
        XNextEvent(display, &x_event);
    }while(x_event.type != MapNotify);
    
    return 0;
}

int Athena_Private_HideWindow(void *handle){
    XEvent x_event;
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    
    XUnmapWindow(display, x_window->window);
    do{
        XNextEvent(display, &x_event);
    }while(x_event.type != UnmapNotify);

    return 0;
}

int Athena_Private_FlipWindow(void *handle){
    XFlush(display);
    return 0;
}

unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to){
    XEvent event;
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    XNextEvent(display, &event);
    switch(event.type){
        case MotionNotify:
            x_window->mouse_x = event.xmotion.x;
            x_window->mouse_y = event.xmotion.y;
        break;
        case UnmapNotify:
            /* Quit outright here? */
        break;
        case DestroyNotify:
            bzero(to, sizeof(struct Athena_Event));
            to->type = athena_quit_event;
            return 1;
        case ButtonPress:

            bzero(to, sizeof(struct Athena_Event));
            /*
            struct Athena_Event {
                enum Athena_EventType type;
                int x, y, w, h, state;
                unsigned which, id;
            };
            */

            /* We might as well update the mouse position while we are here. */
            to->x = x_window->mouse_x = event.xbutton.x;
            to->y = x_window->mouse_y = event.xbutton.y;
            if(event.xbutton.button==1)
                to->which = athena_left_mouse_button;
            else if(event.xbutton.button==2)
                to->which = athena_middle_mouse_button;
            else if(event.xbutton.button==3)
                to->which = athena_right_mouse_button;
            else
                to->which = athena_unknown_mouse_button;
            to->type = athena_click_event;
            return 1;
    }
    return 0;
}

int Athena_Private_GetMousePosition(void *handle, int *x, int *y){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    
    x[0] = x_window->mouse_x;
    y[0] = x_window->mouse_y;
    return 0;
}

int Athena_Private_IsKeyPressed(void *handle, unsigned key){
    return 0;
}
