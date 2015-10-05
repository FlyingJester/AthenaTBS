#include "window.h"
#include "private_window.h"
#include <stdlib.h>

struct Athena_Window {
    void *handle;
    unsigned char is_shown;
    unsigned char dirty;
};

struct Athena_Window *Athena_CreateWindow(unsigned w, unsigned h, const char *title){
    struct Athena_Window *window = malloc(sizeof(struct Athena_Window));
    window->handle = Athena_Private_CreateHandle();
    if(!window)
        return NULL;
    if(Athena_Private_CreateWindow(window->handle, 0, 0, w, h, title)<0)
        return NULL;

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

int Athena_DrawImage(struct Athena_Window *that, int x, int y, unsigned w, unsigned h, enum Athena_ImageFormat format, const void *RGB){
    that->dirty = 1;
    return Athena_Private_DrawImage(that->handle, x, y, w, h, format, RGB);
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

int Athena_Common_Line(void *handle, void *arg, int x0, int y0, int x1, int y1, athena_point_callback callback){
    if(x0==x1){
        if(y0==y1){
            callback(handle, arg, x0, y0);
        }
        else{
            const int direction = (y0<y1)?(1):(-1);
            while(y0!=y1){
                if(y0>=0){
                    callback(handle, arg, x0, y0);
                }
                y0+=direction;
            }
        }
        return 0;
    }
    else if(y0==y1){
        const int direction = (x0<x1)?(1):(-1);
        while(x0!=x1){
            if(x0>=0){
                callback(handle, arg, x0, y0);
            }
            x0+=direction;
        }
        return 0;
    }
    else
        return Athena_Bresenhams(handle, arg, x0, y0, x1, y1, callback);
    
}

int Athena_Bresenhams(void *handle, void *arg, int x0, int y0, int x1, int y1, athena_point_callback callback){
    const float delta_x = x1 - x0,
        delta_y = y1 - y0;

    float error = 0.0f, delta_error = delta_y/delta_x;
    int x = x0, y = y0,  last_y = y;

    const unsigned y_mod = y1>y0;

    if(delta_error<0)
        delta_error=-delta_error;
    
    while(x <= x1){
        unsigned already_set = 1u;

        if(y!=last_y)
            callback(handle, arg, x, y);

        error+=delta_error;
        while(error>=0.5f){
            if(!already_set)
                callback(handle, arg, x, y);

            already_set = 0u;

            y+=y_mod;
            last_y = y_mod;
            error = error-1.0f;
        }
        x++;
    }
    return 0;
}

static float athena_clamp_float(float x){
    if(x>1.0f)
        return 1.0f;
    if(x<0.0f)
        return 0.0f;
    return x;
}

int Athena_Common_ColorToUnsignedByte(const struct Athena_Color *color, unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *alpha){
    if(red)
        red[0] = athena_clamp_float(color->red) * 255.0f;
    if(green)
        green[0] = athena_clamp_float(color->green) * 255.0f;
    if(blue)
        blue[0] = athena_clamp_float(color->blue) * 255.0f;
    if(alpha)
        alpha[0] = athena_clamp_float(color->alpha) * 255.0f;
    return 0;
}

int Athena_Common_ColorToUnsignedShort(const struct Athena_Color *color, unsigned short *red, unsigned short *green, unsigned short *blue, unsigned short *alpha){
    if(red)
        red[0] = athena_clamp_float(color->red) * 65535.0f;
    if(green)
        green[0] = athena_clamp_float(color->green) * 65535.0f;
    if(blue)
        blue[0] = athena_clamp_float(color->blue) * 65535.0f;
    if(alpha)
        alpha[0] = athena_clamp_float(color->alpha) * 65535.0f;
    return 0;
}
