#include "private_window.h"

#include <game/DirectWindow.h>

// We really shouldn't be compiled with an older compiler to begin with...we need epic TCO to work properly...
#if __cplusplus > 199711L
    ATHENA_OVERRIDE override
#else
    ATHENA_OVERRIDE
#endif

#include <vector>
#include <algorithm>

class Athena_Window : public BDirectWindow{
    
    /*
    typedef struct {
       direct_buffer_state   buffer_state;
       direct_driver_state   driver_state;
       void*                 bits;
       void*                 pci_bits;
       int32                 bytes_per_row;
       uint32                bits_per_pixel;
       color_space           pixel_format;
       buffer_layout         layout;
       buffer_orientation    orientation;
       uint32                _reserved[9];
       uint32                _dd_type_;
       uint32                _dd_token_;
       uint32                clip_list_count;
       clipping_rect         window_bounds;
       clipping_rect         clip_bounds;
       clipping_rect         clip_list[1];
    } direct_buffer_info;
    */
    
    uint8_t *screen;
    uint32_t pitch, depth; // both in bytes.
    color_space format;
    clipping_rect bounds;

    std::vector<clipping_rect> clip_list;
    
    bool connected, connection_disabled;
    
    static void ConvertColorSpaces(const uint32_t *in, void *out, size_t num_pixels, color_space c);
    
public:
    Athena_Window(int x, int y, unsigned w, unsigned h, const char *title)
      : BDirectWindow(BRect(x, y, x + w, y + h), title, B_TITLED_WINDOW, B_NOT_RESIZABLE|B_QUIT_ON_WINDOW_CLOSE, B_CURRENT_WORKSPACE)
      , locker("Athena Window Locker"){
        
        
        
    }

    ~Athena_Window(){
        fConnectionDisabled = false;
        Hide();
        Sync();
    }

    virtual void DirectConnected(direct_buffer_info* info) ATHENA_OVERRIDE;


};

void Athena_Window::ConvertColorSpaces(const uint32_t *in, void *out, size_t num_pixels, color_space c){
    switch(c){
        case B_RGB32:
        case B_RGBA32:
        case B_RGB32_BIG:
        case B_RGBA32_BIG:
            std::copy(in, in + num_pixels, static_cast<uint32_t *>(out));
            break;
        case B_RGB24:
        case B_RGB24_BIG:
            for(int i = 0; i<num_pixels; i++){
                pixel = static_cast<uint8_t *>(out) + (3 * i);
                pixel[0] = in[i] & 0xFF;
                pixel[1] = (in[i] >> 8) & 0xFF;
                pixel[2] = (in[i] >> 16) & 0xFF;
            }
            break;
        default:
            // ...
            break;
    }
}

void Athena_Window::DirectConnected(direct_buffer_info *info){
    switch(info->buffer_state & B_DIRECT_MODE_MASK){
        case B_DIRECT_START:
            connected = true;
        case B_DIRECT_MODIFY:
            clip_list.clear();
            clip_list.insert(clip_list.end(), info->clip_list, info->clip_list + info->clip_list_count);
            
            screen = reinterpret_cast<uint8_t *>(info->bits);
            pitch = info->bytes_per_row;
            depth = info->bits_per_pixel >> 3;
            format = info->pixel_format;
            bounds = info->window_bounds;
            
        case B_DIRECT_STOP:
            connected = false;
    }
}

void *Athena_Private_CreateHandle();
int Athena_Private_DestroyHandle(void *);
int Athena_Private_CreateWindow(void *handle, int x, int y, unsigned w, unsigned h, const char *title);
int Athena_Private_ShowWindow(void *);
int Athena_Private_HideWindow(void *);

/* Neither the BeBook nor the Haiku docs mention the composition of a clipping_rect :( */
int Athena_Private_DrawImage(void *handle, int x, int y, unsigned w, unsigned h, unsigned format, const void *RGB){
    /* In terms of RGB */
    const int starting_x = std::max(0, -x),
        starting_y = std::max(0, -y),
        ending_x = std::min<int>(w, static_cast<int>(bounds.right) - x),
        ending_y = std::min<int>(h, static_cast<int>(bounds.bottom) - y);
    
    uint8_t *row_start = screen + (starting_y * pitch);
    
    for(int i = starting_y; i < ending_y; i++){
        for(int e = starting_x; e < ending_x; e++){
            ConvertColorSpaces(static_cast<uint32_t *>(RGB) + e + (i * w), row_start + (x * depth), format);
        }
        row_start += pitch;
    }
}

int Athena_Private_DrawRect(void *that, int x, int y, unsigned w, unsigned h, const struct Athena_Color *color){
    const int starting_x = std::max(0, -x),
        starting_y = std::max(0, -y),
        ending_x = std::min<int>(w, static_cast<int>(bounds.right) - x),
        ending_y = std::min<int>(h, static_cast<int>(bounds.bottom) - y);
    
    uint8_t *row_start = screen + (starting_y * pitch);
    
    for(int i = starting_y; i < ending_y; i++){
        ConvertColorSpaces(static_cast<uint32_t *>(RGB) + e + (i * w), row_start + (x * depth), ending_x - starting_x, format);
        row_start += pitch;
    }
}

int Athena_Private_DrawLine(void *that, int x1, int y1, int x2, int y2, const struct Athena_Color *color){    
    return 0;
}

int Athena_Private_FlipWindow(void *handle){

}

unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to);

/* Athena_Common functions are common to all backends, but are private to this library.
 * These are intended to be used from the Athena_Private functions.
 * No Athena_Common function will call any Athena_Private function, to categorically avoid infinite mutual recursion.
 */
int Athena_Common_Line(void *handle, void *arg, int x1, int y1, int x2, int y2, athena_point_callback callback);
int Athena_Common_ColorToUnsignedByte(const struct Athena_Color *color, unsigned char *red, unsigned char *greeb, unsigned char *blue, unsigned char *alpha);
int Athena_Common_ColorToUnsignedShort(const struct Athena_Color *color, unsigned short *red, unsigned short *greeb, unsigned short *blue, unsigned short *alpha);

int Athena_Private_IsKeyPressed(void *handle, unsigned key);

