#include "window.h"
#include "private_window.h"

/* This is necessary on some glibc setups to get shm */
#define _XOPEN_SOURCE 500

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __linux__
#include <errno.h>
#else
#include <sys/errno.h>
#endif
/*
#define PRINT_ALL_X_MODES 1
*/

#define ATHENA_X_EVENT_MASK (StructureNotifyMask|KeyPressMask|ButtonPress|ExposureMask|PointerMotionMask)

static Display *display = NULL;

#define ATHENA_VERIFY(A_HANDLE)\
    A_HANDLE;\
    if(!display)\
        return -1;\
    if(!A_HANDLE)\
        return -1

#define ATHENA_MAX_KEYS 100

/* This may later change to something else to support unicode codepoints. */
typedef unsigned char athena_key_t;

struct Athena_X_Window {
    Window root_window, window;
    Colormap color_map;
    GC gc;
    int screen;

    int mouse_x, mouse_y;

    XVisualInfo *visinfo;
    int framebuffer_info;
    XImage *framebuffer;
    XShmSegmentInfo shminfo;
    unsigned was_attached;

    unsigned w, h;

    athena_key_t keys[ATHENA_MAX_KEYS];
};

static const XVisualInfo *athena_x11_visinfo(const struct Athena_X_Window *x_window){
    return x_window->visinfo + x_window->framebuffer_info;
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

#if (defined __APPLE__) || (defined_WIN32)
#define ATHENA_ALLOW_32_BITDEPTH
#endif

int Athena_Private_CreateWindow(void *handle, int x, int y, unsigned w, unsigned h, const char *title){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    x_window->w = w;
    x_window->h = h;
    x_window->was_attached = 0;
    {
        /* Create our shared memory image on the X11 side to push our updates to. */
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
#ifdef ATHENA_ALLOW_32_BITDEPTH
            if(x_window->visinfo[i].class == TrueColor && x_window->visinfo[i].depth==32){
                x_window->framebuffer_info = i;
                break;
            }
#endif
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
        
        if(x_window->framebuffer_info==-1){
            puts("[Athena_Private_CreateWindow]Couldn't find a reliable framebuffer visual, defaulting to visual 0");
            x_window->framebuffer_info = 0;
        }
#ifndef NDEBUG
        else{
            printf("[Athena_Private_CreateWindow]Used framebuffer visual %i\n", x_window->framebuffer_info);
        }
#endif
        /* Actually create the image, as well as the shared memory segment to copy to. */
        x_window->framebuffer = XShmCreateImage(display, athena_x11_visinfo(x_window)->visual, athena_x11_visinfo(x_window)->depth, ZPixmap, NULL, &x_window->shminfo, w, h);
        printf("[Athena_Private_CreateWindow]Created image of size %i (pitch) x %i (height) = %i\n", x_window->framebuffer->bytes_per_line, x_window->framebuffer->height, x_window->framebuffer->bytes_per_line * x_window->framebuffer->height);
        x_window->shminfo.shmid = shmget(IPC_PRIVATE, x_window->framebuffer->bytes_per_line * x_window->framebuffer->height, IPC_CREAT|0777);
        
        if(x_window->shminfo.shmid == -1){
            perror("x_window->shminfo.shmid");
            switch(errno){
                case EACCES:
                    puts("No permission to make shared pages");
                    break;
                case EEXIST:
                    puts("Segment already exists");
                    break;
                case ENFILE:
                    puts("System file limit reached");
                    break;
                case ENOENT:
                    puts("Tried to access a key that was not yet created");
                    break;
                case ENOMEM:
                    puts("No memory");
                    break;
                case ENOSPC:
                    puts("All shared memory IDs are taken.");
                    break;
            
            }
        }

        x_window->shminfo.shmaddr = x_window->framebuffer->data = shmat(x_window->shminfo.shmid, 0, 0); 
        x_window->shminfo.readOnly = True;
        

        /* Create the window */
        x_window->window = XCreateSimpleWindow(display, x_window->root_window, x, y, w, h, 0, black, black);
    }

    XSelectInput(display, x_window->window, ATHENA_X_EVENT_MASK);
    x_window->gc = XCreateGC(display, x_window->window, 0, NULL);

    if(title)
        XStoreName(display, x_window->window, title);

    memset(x_window->keys, 0, ATHENA_MAX_KEYS * sizeof(athena_key_t));

    return 0;
}

int Athena_Private_Update(void *handle, unsigned format, const void *RGBA, unsigned w, unsigned h){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    if(x_window->was_attached){
        XShmDetach(display, &x_window->shminfo);
        x_window->was_attached = 0;
    }

    if(athena_x11_visinfo(x_window)->depth==32){
        memcpy(x_window->shminfo.shmaddr, RGBA, w * h << 2);
    }
    else if(athena_x11_visinfo(x_window)->depth==24){
        memcpy(x_window->shminfo.shmaddr, RGBA, w * h << 2);
/*
        unsigned i;
        const unsigned num_pixels = w * h;
        const unsigned int *in = RGBA;
        for(i = 0; i<num_pixels; i++){
            unsigned char *pixel = ((unsigned char *)(x_window->shminfo.shmaddr)) + (i * 4);
            pixel[2] = in[i] & 0xFF;
            pixel[1] = (in[i] >> 8) & 0xFF;
            pixel[0] = (in[i] >> 16) & 0xFF;
        }
*/
    }
    else{
        memcpy(x_window->shminfo.shmaddr, RGBA, w * h * athena_x11_visinfo(x_window)->depth >> 3);
    }
    return 0;
}

int Athena_Private_DestroyHandle(void *handle){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    if(x_window->was_attached)
        XShmDetach(display, &x_window->shminfo);
    XDestroyImage(x_window->framebuffer);
    shmdt(x_window->shminfo.shmaddr);
    shmctl(x_window->shminfo.shmid, IPC_RMID, 0);

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
    puts("[Athena_Private_ShowWindow]Window Mapped");

    return 0;
}

int Athena_Private_HideWindow(void *handle){
    XEvent x_event;
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    
    XUnmapWindow(display, x_window->window);
    do{
        XNextEvent(display, &x_event);
    }while(x_event.type != UnmapNotify);
    puts("[Athena_Private_ShowWindow]Window Unmapped");

    return 0;
}

int Athena_Private_FlipWindow(void *handle){
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    
    if(x_window->was_attached || (XShmAttach(display, &x_window->shminfo)!=0)){
        if(!x_window->was_attached){
            XSync(display, False);
            x_window->was_attached = 1;
        }
        XShmPutImage(display, x_window->window, x_window->gc, x_window->framebuffer, 0, 0, 0, 0, x_window->w, x_window->h, False);
        XSync(display, False);
    }
    return 0;
}

static int athena_x_prepare_key(unsigned key){
    if(key>='0' && key<='9')
        return ('z' - 'a') + (key - '0');

    if(key >= 'A' && key<='Z')
        key -= 'A' - 'a';
    if(key>= 'a')
        key -= 'a';
    if(key < ATHENA_MAX_KEYS)
        return key;
    else
        return -1;
}

unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to){
    XEvent event;
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);

    if(XCheckMaskEvent(display, ATHENA_X_EVENT_MASK, &event)==False)
        return 0;

    switch(event.type){
        case MotionNotify:
            x_window->mouse_x = event.xmotion.x;
            x_window->mouse_y = event.xmotion.y;
        break;
        case UnmapNotify:
            /* Quit outright here? */
        break;
        case DestroyNotify:
            memset(to, 0, sizeof(struct Athena_Event));
            to->type = athena_quit_event;
            return 1;
        case ButtonPress:

            memset(to, 0, sizeof(struct Athena_Event));

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
        case KeyPress:
            {   
                KeySym sim;
                XComposeStatus compose;
                char buffer[16];
                int len = XLookupString(&event.xkey, buffer, sizeof(buffer), &sim, &compose);
                if(!(buffer[0]!=0 && buffer[1]==0 && len))
                    break;
                else{
                    int key = athena_x_prepare_key(buffer[0]);
                    if(key>=0){
                        printf("Parsed %s as %i\n", buffer, key);
                        memset(to, 0, sizeof(struct Athena_Event));

                        x_window->keys[key] = 1;

                        to->which = key + 'a';
                        to->type = athena_key_event;
                        return 1;
                    }
                    else{
                        printf("Unable to parse %s\n", buffer);
                    }
                }
            }
    

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
    struct Athena_X_Window * const x_window = ATHENA_VERIFY(handle);
    
    {
        const int k = athena_x_prepare_key(key);
        if(k>=0)
            return x_window->keys[k];
    }
    return 0;
}
