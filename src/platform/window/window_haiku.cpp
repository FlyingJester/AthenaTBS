#include "private_window.h"

#include <game/DirectWindow.h>
#include <Application.h>

// We really shouldn't be compiled with an older compiler to begin with...we need epic TCO to work properly...
#if __cplusplus > 199711L
    #define ATHENA_OVERRIDE override
#else
    #define ATHENA_OVERRIDE
#endif

#include <vector>
#include <queue>
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
    std::queue<Athena_Event> queued_events;
    
    bool connected, connection_disabled;
    
    BPoint mouse_location;
    
    static void ConvertColorSpaces(const uint32_t *in, void *out, size_t num_pixels, color_space c);
    
public:
    Athena_Window(int x, int y, unsigned w, unsigned h, const char *title)
      : BDirectWindow(BRect(x, y, x + w, y + h), title, B_TITLED_WINDOW, B_NOT_RESIZABLE|B_QUIT_ON_WINDOW_CLOSE, B_CURRENT_WORKSPACE){
        
        
        
    }

    ~Athena_Window(){
        connection_disabled = false;
        Hide();
        Sync();
    }
    
	int DrawImage(int x, int y, unsigned w, unsigned h, unsigned format, const void *RGB);
	int DrawRect(int x, int y, unsigned w, unsigned h, const struct Athena_Color *color);

	virtual void DirectConnected(direct_buffer_info* info) ATHENA_OVERRIDE;
	virtual void MessageReceived(BMessage* message) ATHENA_OVERRIDE;
    
	void GetMousePosition(int &x, int &y) const;
	void GetMousePosition(int *x, int *y) const;
    
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
            for(unsigned i = 0; i<num_pixels; i++){
                uint8_t *pixel = static_cast<uint8_t *>(out) + (3 * i);
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

void Athena_Window::MessageReceived(BMessage* message){
    if(message->what == B_MOUSE_DOWN){
        struct Athena_Event event = { athena_click_event };
        int32 type;

        message->FindInt32("buttons", &type);

        if(type==B_PRIMARY_MOUSE_BUTTON)
            event.which = athena_left_mouse_button;
        else if(type==B_SECONDARY_MOUSE_BUTTON)
            event.which = athena_right_mouse_button;
        else if(type==B_TERTIARY_MOUSE_BUTTON)
            event.which = athena_middle_mouse_button;
        else
            event.which = athena_unknown_mouse_button;
    
        BPoint point;
        
        message->FindPoint("where", &point);
        
        event.x = point.x;
        event.y = point.y;
    
        queued_events.push(event);
    }
    else if(message->what == B_MOUSE_MOVED){
        message->FindPoint("where", &mouse_location);
    }
}

void Athena_Window::GetMousePosition(int &x, int &y) const{
    GetMousePosition(&x, &y);
}

void Athena_Window::GetMousePosition(int *x, int *y) const{
    x[0] = mouse_location.x;
    y[0] = mouse_location.y;
}

struct Athena_WindowHandle{
    Athena_Window *window;
};

struct Athena_Thread{
	thread_id thread;
	void *arg;
	void (*callback)(void *);
};

athena_thread_wrapper(void *data){
	Athena_Thread *thread = static_cast<Athena_Thread *>(data);
	thread->callback(thread->arg);
	return 0;
}

class Athena_Application : public BApplication {
    thread_id runner_thread;

    int32 RunnerThread(){
        Run();
    }
    
    static int32 athena_runner_thread(void *that){
        return static_cast<Athena_Application *>(that)->RunnerThread();
    }
    static bool exists;
    static BLocker ensure_locker;

    Athena_Application(){
        runner_thread = spawn_thread(athena_runner_thread, "athena_app_runner", B_NORMAL_PRIORITY, this);
        resume_thread(runner_thread);
        exists = true;
    }

public:

    static void EnsureApplication(){
        ensure_locker.lock();
        if(!exists)
            new Athena_Application();
        ensure_locker.unlock();
    }
};

bool Athena_Application::exists = false;
ensure_locker Athena_Application::ensure_locker;

void *Athena_Private_CreateHandle(){

    Athena_Application::EnsureApplication();

    Athena_WindowHandle *handle = new Athena_WindowHandle();
    return handle;
}

int Athena_Private_DestroyHandle(void *that){
    Athena_WindowHandle *const handle = static_cast<Athena_WindowHandle *>(that);
    delete handle->window;
    delete handle;
    return 0;
}

int Athena_Private_CreateWindow(void *that, int x, int y, unsigned w, unsigned h, const char *title){
    Athena_WindowHandle *const handle = static_cast<Athena_WindowHandle *>(that);
    handle->window = new Athena_Window(x, y, w, h, title);
    return 0;
}

int Athena_Private_ShowWindow(void *that){
    Athena_WindowHandle *const handle = static_cast<Athena_WindowHandle *>(that);
    handle->window->Show();
    return 0;
}

int Athena_Private_HideWindow(void *that){
    Athena_WindowHandle *const handle = static_cast<Athena_WindowHandle *>(that);
    handle->window->Hide();
    return 0;

}

int Athena_Window::DrawImage(int x, int y, unsigned w, unsigned h, unsigned format_x, const void *RGB){
    /* In terms of RGB */
    const int starting_x = std::max(0, -x),
        starting_y = std::max(0, -y),
        ending_x = std::min<int>(w, static_cast<int>(bounds.right) - x),
        ending_y = std::min<int>(h, static_cast<int>(bounds.bottom) - y);
    
    uint8_t *row_start = screen + (starting_y * pitch);
    
    for(int i = starting_y; i < ending_y; i++){
        ConvertColorSpaces(static_cast<const uint32_t *>(RGB) + starting_x + (i * w), row_start + (x * depth), ending_x - starting_x, this->format);
        row_start += pitch;
    }
    return 0;
}

/* Neither the BeBook nor the Haiku docs mention the composition of a clipping_rect :( */
int Athena_Private_DrawImage(void *that, int x, int y, unsigned w, unsigned h, unsigned format, const void *RGB){
	return static_cast<Athena_WindowHandle *>(that)->window->DrawImage(x, y, w, h, format, RGB);
}

int Athena_Private_DrawRect(void *handle, int x, int y, unsigned w, unsigned h, unsigned format, const struct Athena_Color *color){
	return 0;
}

int Athena_Private_DrawLine(void *handle, int x1, int y1, int x2, int y2, const struct Athena_Color *color){    
	return 0;
}

int Athena_Private_FlipWindow(void *handle){
	return 0;
}

unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to){
	return 0;
}

int Athena_Private_IsKeyPressed(void *handle, unsigned key){
    return 0;
}

int Athena_Private_GetMousePosition(void *handle, int *x, int *y){
    static_cast<Athena_WindowHandle *>(that)->window->GetMousePosition(x, y);
    return 0;
}
