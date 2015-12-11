#include "private_window.h"
#include <game/DirectWindow.h>
#include <app/Application.h>
#include <support/Locker.h>
#include <kernel/OS.h>
#include <queue>
#include <cstring>
#include <cstdlib>
#include <cstdio>

// Just in case.
#if __cplusplus > 199711L
    #define ATHENA_OVERRIDE override
#else
    #define ATHENA_OVERRIDE
    #define nullptr NULL
#endif

////////////////////////////////////////////////
// Athena_Application : BApplication

class Athena_Application : public BApplication{
public:
    Athena_Application()
      : BApplication("application/net.flyingjester.athena"){
        
    }

    virtual ~Athena_Application(){}
/*
    virtual void MessageRecieved(BMessage *message) ATHENA_OVERRIDE {
        // Once we know what we are looking for, we'll get some events from this.
        BApplication::MessageRecieved(message);
    }*/

    static thread_id msg_thread;
    static int32 msg_handler_callback(void *data){
        be_app->Run();

        //...
        delete be_app;
       	return 0;
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

	int32 last_mouse_x, last_mouse_y;

    sem_id gfx_sem;
    
    std::queue<Athena_Event> athena_events;
    sem_id event_sem;

public:
    Athena_DirectWindow(BRect frame, const char* title)
      : BDirectWindow(frame, title, B_TITLED_WINDOW, 
            B_NOT_RESIZABLE|B_QUIT_ON_WINDOW_CLOSE, B_CURRENT_WORKSPACE){

    }
    
    void GetMousePosition(int *x, int *y){
        if(acquire_sem(event_sem)!= B_NO_ERROR)
        	return;
        x[0] = last_mouse_x;
        y[0] = last_mouse_y;
        release_sem(event_sem);
    }
    
    int GetAthenaEvent(Athena_Event *to){
        if(acquire_sem(event_sem)!= B_NO_ERROR)
        	return 0;

        const int ev = !athena_events.empty();
        if(ev){
            to[0] = athena_events.front();
            athena_events.pop();
        }
        release_sem(event_sem);
        return ev;
    }
    
    virtual void MessageReceived(BMessage *msg) ATHENA_OVERRIDE {
        int32 type;
        BPoint point;

        Athena_Event event;
        bool populated;
        memset(&event, 0, sizeof(Athena_Event));

        msg->FindPoint("where", &point);

        switch(msg->what){
            case B_MOUSE_DOWN:
                populated = true;

                msg->FindInt32("buttons", &type);
                if(type==B_PRIMARY_MOUSE_BUTTON)
                    event.which = athena_left_mouse_button;
                else if(type==B_SECONDARY_MOUSE_BUTTON)
                    event.which = athena_right_mouse_button;
                else if(type==B_TERTIARY_MOUSE_BUTTON)
                    event.which = athena_middle_mouse_button;
                else
                    event.which = athena_unknown_mouse_button;

            case B_MOUSE_MOVED: // Fallthrough
            case B_MOUSE_UP: // Fallthrough
            	event.x = last_mouse_x = point.x;
            	event.y = last_mouse_y = point.y;
            	break;
            	
        }

        if(!populated || acquire_sem(event_sem)!=B_NO_ERROR)
            return;

		athena_events.push(event);

        release_sem(event_sem);
    }
    
    virtual void DirectConnected(direct_buffer_info* info) ATHENA_OVERRIDE {
        acquire_sem(gfx_sem);
        switch(info->buffer_state & B_DIRECT_MODE_MASK){
            case B_DIRECT_START:
                connected = true;
            case B_DIRECT_MODIFY: // Fallthrough
                bits = (int8 *)info->bits;
                bits_per_pixel = info->bits_per_pixel;
                byte_pitch = info->bytes_per_row;
                format = info->pixel_format;
                bounds = info->window_bounds;
                break;
            case B_DIRECT_STOP:
                connected = false;
                bits = nullptr;
                break;
        }

        release_sem(gfx_sem);
    }

    void Blit(unsigned f, const int8 *RGB, int32 a_w, int32 a_h){
        acquire_sem(gfx_sem);
        if(connected && bits){
            const int32 l_w = (bounds.right - bounds.left) + 1,
                l_h = (bounds.bottom - bounds.top) + 1;
            for(int l_y = 0; l_y<l_h && l_y < a_h; l_y++){
                int8 * const to = bits + (((bounds.top + l_y) * byte_pitch) + bounds.left) * (bits_per_pixel>>3);
                switch(format){
                    case B_NO_COLOR_SPACE:
                    	break;
                    case B_RGB32:
                    case B_RGBA32:
                    case B_RGB32_BIG:
                    case B_RGBA32_BIG:
                        memcpy(to, RGB + (a_w * l_y * 4), ((l_w<a_w)?l_w:l_w)*4);
                        break;
                    default:
                        {
                            static bool warned = false;
                            if(warned){
                                warned = true;
                                fputs("[Athena_DirectWindow::Blit] Could not use color space.", stderr);
                            }
                        }
                }
            }
        }
        release_sem(gfx_sem);
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
    return 0;
}

int Athena_Private_CreateWindow(void *a, int x, int y, unsigned w, unsigned h, const char *title){
    Athena_DirectWindow * const window = 
        static_cast<Athena_DirectWindow **>(a)[0] = 
        new Athena_DirectWindow(BRect(x, y, w, h), title);
    return window==nullptr;
}

int Athena_Private_Update(void *a, unsigned format, const void *RGB, unsigned w, unsigned h){
    static_cast<Athena_DirectWindow **>(a)[0]->Blit(format, (int8 *)RGB, w, h);
    return 0;
}

int Athena_Private_FlipWindow(void *a){
	return a==nullptr;
}

int Athena_Private_ShowWindow(void *a){
    static_cast<Athena_DirectWindow **>(a)[0]->Show();
    return 0;
}

int Athena_Private_HideWindow(void *a){
    static_cast<Athena_DirectWindow **>(a)[0]->Hide();
    return 0;
}

int Athena_Private_GetMousePosition(void *a, int *x, int *y){
    static_cast<Athena_DirectWindow **>(a)[0]->GetMousePosition(x, y);
    return 0;
}

unsigned Athena_Private_GetEvent(void *a, struct Athena_Event *to){
    return static_cast<Athena_DirectWindow **>(a)[0]->GetAthenaEvent(to);
}

int Athena_Private_IsKeyPressed(void *handle, unsigned key){
    return 0;
}
