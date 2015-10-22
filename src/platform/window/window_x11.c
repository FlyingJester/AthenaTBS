#include "private_window.h"
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <stdlib.h>

struct Athena_XCBHandle{
    Display *display;
    xcb_connection_t *connection;
    xcb_window_t window;
    
    const xcb_setup_t *setup;
    xcb_screen_t *screen;
    
    xcb_gcontext_t ctx;
};

/*
static void athena_push_values(struct Athena_XCBHandle *handle, uint32_t mask, uint32_t *value_list){
    xcb_change_gc(handle->connection, handle->ctx, mask, value_list);
}
*/

static void athena_flush_handle(void *handle){
   xcb_flush(((struct Athena_XCBHandle *)handle)->connection);
}

void *Athena_Private_CreateHandle(){
    struct Athena_XCBHandle *handle = calloc(1, sizeof(struct Athena_XCBHandle));
    handle->display = XOpenDisplay(NULL);
    handle->connection = XGetXCBConnection(handle->display);
    handle->window = xcb_generate_id(handle->connection);
    handle->ctx = xcb_generate_id(handle->connection);
    
    handle->setup = xcb_get_setup(handle->connection);
    handle->screen = xcb_setup_roots_iterator(handle->setup).data;
    
    return handle;
}

int Athena_Private_DestroyHandle(void *arg){
    struct Athena_XCBHandle *handle = arg;
    
    xcb_disconnect(handle->connection);
    
    return 0;
}

int Athena_Private_CreateWindow(void *arg, int x, int y, unsigned w, unsigned h, const char *title){
    struct Athena_XCBHandle *handle = arg;
    xcb_create_window(handle->connection, 
        4,
        handle->window,
        handle->screen->root,
        x, y, w, h,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        handle->screen->root_visual,
        0, NULL);
    
    {
        uint32_t value = handle->screen->black_pixel;
        xcb_create_gc(handle->connection, handle->ctx, handle->window, XCB_GC_FOREGROUND, &value);
        
    }
    return 0;
}

int Athena_Private_ShowWindow(void *arg){
   struct Athena_XCBHandle *handle = arg;
   xcb_map_window(handle->connection, handle->window);
   athena_flush_handle(arg);
   return 0;
}

int Athena_Private_HideWindow(void *arg);
int Athena_Private_DrawImage(void *arg, int x, int y, unsigned w, unsigned h, unsigned format, const void *RGB){
    struct Athena_XCBHandle *handle = arg;
    xcb_pixmap_t pixmap = xcb_generate_id(handle->connection);
    xcb_create_pixmap(handle->connection, 32, pixmap, handle->window, w, h);
    xcb_free_pixmap(handle->connection, pixmap);
}

int Athena_Private_DrawRect(void *arg, int x, int y, unsigned w, unsigned h, const struct Athena_Color *color){

/*
    xcb_change_gc(xcb_connection_t *c, xcb_gcontext_t gc, uint32_t value_mask, const uint32_t *value_list);
*/

    struct Athena_XCBHandle *handle = arg;
    xcb_rectangle_t rectangle;
    rectangle.x = x; rectangle.y = y; rectangle.width = w; rectangle.height = h;
    xcb_poly_fill_rectangle(handle->connection, handle->window, handle->ctx, 1, &rectangle);
    return 0;
}

int Athena_Private_DrawLine(void *arg, int x1, int y1, int x2, int y2, const struct Athena_Color *color){
    struct Athena_XCBHandle *handle = arg;
    xcb_segment_t segment;
    segment.x1 = x1; segment.y1 = y1; segment.x2 = x2; segment.y2 = y2;
    xcb_poly_segment(handle->connection, handle->window, handle->ctx, 1, &segment);
    return 0;
}

int Athena_Private_FlipWindow(void *arg){

   athena_flush_handle(arg);
   return 0;
}

unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to){
    return 0;
}

/* Athena_Common functions are common to all backends, but are private to this library.
 * These are intended to be used from the Athena_Private functions.
 * No Athena_Common function will call any Athena_Private function, to categorically avoid infinite mutual recursion.
 */
int Athena_Common_Line(void *handle, void *arg, int x1, int y1, int x2, int y2, athena_point_callback callback);
int Athena_Common_ColorToUnsignedByte(const struct Athena_Color *color, unsigned char *red, unsigned char *greeb, unsigned char *blue, unsigned char *alpha);
int Athena_Common_ColorToUnsignedShort(const struct Athena_Color *color, unsigned short *red, unsigned short *greeb, unsigned short *blue, unsigned short *alpha);

