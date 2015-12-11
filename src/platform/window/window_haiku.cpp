#include "private_window.h"
#include <game/DirectWindow.h>
#include <app/Application.h>
#include <kernel/OS.h>
#include <queue>

// Just in case.
#if __cplusplus > 199711L
    #define ATHENA_OVERRIDE override
#else
    #define ATHENA_OVERRIDE
    #define nullptr NULL
#endif

static std::queue<Athena_Event> athena_haiku_events;
static sem_id athena_haiku_event_semaphore;

////////////////////////////////////////////////
// Athena_Application : BApplication

class Athena_Application : public BApplication{
public:
    Athena_Application()
      : BApplication("application/net.flyingjester.athena"){
        
    }

    virtual ~Athena_Application(){}

    virtual void MessageRecieved(BMessage *message) ATHENA_OVERRIDE {
        // Once we know what we are looking for, we'll get some events from this.
        BApplication::MessageRecieved(message);
    }

    static thread_id msg_thread;
    static int32 msg_handler_callback(void *data){
        be_app->Run();

        //...
        delete be_app;
    }
};

thread_id Athena_Application::msg_thread;

////////////////////////////////////////////////
// Athena_DirectWindow : BDirectWindow

class Athena_DirectWindow : public BDirectWindow {

    bool connected;
    int8 *bits;
    int32 bits_per_pixel, byte_pitch;
    color_space format;
    clipping_rect bounds;

    BLocker locker;

public:
    Athena_DirectWindow(BRect frame, const char* title)
      : BDirectWindow(frame, title, B_TITLED_WINDOW, 
            B_NOT_RESIZABLE|B_QUIT_ON_WINDOW_CLOSE, B_CURRENT_WORKSPACE){

    }
    
    virtual void DirectConnected(direct_buffer_info* info) ATHENA_OVERRIDE {
        locker.lock();
        switch(info->buffer_state & B_DIRECT_MODE_MASK){
            case B_DIRECT_START:
                connected = true;
            case B_DIRECT_MODIFY: // Fallthrough
                buffer = info->bits;
                bits_per_pixel = info->bits_per_pixel;
                byte_pitch = info->bytes_per_row;
                format = info->pixel_format;
                bounds = info->window_bounds;
                break;
            case B_DIRECT_STOP:
                connected = false;
                bits = nullptr;
        }

        locker.unlock();
    }

    void Blit(unsigned f, const int8 *RGB, unsigned a_w, unsigned a_h){
        locker.lock();
        if(connected && bits){
            const int32 l_w = (bounds.right - bounds.left) + 1,
                l_h = (bounds.bottom - bounds.top) + 1;
            for(int l_y = 0; l_y<l_h && l_y < a_h; l_y++){
                int8 *to = bits + (((bounds.top + l_y) * byte_pitch) + bounds.left) * (bits_per_pixel>>3)
                switch(format){
                    case B_RGB32:
                    case B_RGBA32:
                    case B_RGB32_BIG:
                    case B_RGBA32_BIG:
                        memcpy(to, RGB + (w * l_y * 4), ((l_w<a_h)?l_w:l_h)*4);
                        break;
                }
            }
        }
        locker.unlock();
    }

};

void *Athena_Private_CreateHandle(){
    if(!be_app){
        new Athena_Application();
        Athena_Application::msg_thread = 
            spawn_thread(Athena_Application::msg_handler_callback, "athena_haiku_app", B_NORMAL_PRIORITY, nullptr);
        resume_thread(Athena_Application::msg_thread);
    }
    
    return calloc(sizeof(void *), 1);
}

int Athena_Private_DestroyHandle(void *a){
    if(Athena_DirectWindow *window = *static_cast<Athena_DirectWindow **>(a))
        delete window;
    free(a);
}

int Athena_Private_CreateWindow(void *handle, int x, int y, unsigned w, unsigned h, const char *title){
    Athena_DirectWindow * const window = 
        static_cast<Athena_DirectWindow **>(a)[0] = 
        new Athena_DirectWindow(BRect(x, y, w, h), title);
}

int Athena_Private_Update(void *handle, unsigned format, const void *RGB, unsigned w, unsigned h){
    static_cast<Athena_DirectWindow **>(handle)[0]->Blit(format, RGB, w, h);
}

int Athena_Private_ShowWindow(void *a){
    static_cast<Athena_DirectWindow **>(a)[0]->Show();
}

int Athena_Private_HideWindow(void *a){
    static_cast<Athena_DirectWindow **>(a)[0]->Hide();
}

unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to){
    if(acquire_sem(athena_haiku_event_semaphore) == B_NO_ERROR){
        const bool n = !athena_haiku_events.empty();
        if(n){
            to[0] = athena_haiku_events.front();
            athena_haiku_events.pop_front();
        }
        return n;
        release_sem(athena_haiku_event_semaphore);
    }
    else{
        static int already_warned = 0;
        if(already_warned<16){
            fputs("[Athena_Private_GetEvent] Error locking the event semaphore.\n", stderr);
            if(++already_warned==16)
                fputs("[Athena_Private_GetEvent] Suppressing further warnings.\n", stderr);
        }
        return 0;
    }
}
