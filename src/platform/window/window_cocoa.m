#include "window.h"
#include "window_defs.h"
#import <Cocoa/Cocoa.h>
#import <Foundation/NSLock.h>
#include <stdio.h>

struct Athena_EventList{
    struct Athena_Event event;
    struct Athena_EventList *next;
};

/* Responder to export events to Athena proper. */
@interface Athena_WindowListener : NSResponder <NSWindowDelegate>
    {
        struct Athena_EventList *events;
        NSLock *lock;
    }
     - (id) init;
     - (void) mouseDown:(NSEvent *) event;
     - (void) rightMouseDown:(NSEvent *) event;
     - (void) otherMouseDown:(NSEvent *) event;
     
     - (BOOL) popEvent: (struct Athena_Event *)to;
     - (void) pushEvent: (struct Athena_Event *)event;
     
     + (void) mousePosition: (struct Athena_Event *)to
                whichWindow:(NSWindow *)window;

@end

@implementation Athena_WindowListener

      - (id) init{
         self = [super init];
         
         lock = [[NSLock alloc] init];
         events = NULL;
         
         return self;
     }
     
      - (void) mouseDown:(NSEvent *) event{
         struct Athena_Event athena_event = { athena_click_event };
         athena_event.which = athena_left_mouse_button;

         [Athena_WindowListener mousePosition:&athena_event whichWindow:[event window]];
         
         [self pushEvent:&athena_event];
     }
          
      - (void) rightMouseDown:(NSEvent *) event{
         struct Athena_Event athena_event = { athena_click_event };
         athena_event.which = athena_right_mouse_button;
         
         [Athena_WindowListener mousePosition:&athena_event whichWindow:[event window]];
         
         [self pushEvent:&athena_event];
     }
          
      - (void) otherMouseDown:(NSEvent *) event{
         struct Athena_Event athena_event = { athena_click_event };
         athena_event.which = athena_middle_mouse_button;
         
         [Athena_WindowListener mousePosition:&athena_event whichWindow:[event window]];
         
         [self pushEvent:&athena_event];
     }
     
      - (void) pushEvent: (struct Athena_Event *)event{
         
         [lock lock];
         {
             struct Athena_EventList **next_event = &events;
             while(next_event[0])
                 next_event = &(next_event[0]->next);

             next_event[0] = malloc(sizeof(struct Athena_EventList));
             next_event[0]->next = NULL;
             memcpy(&next_event[0]->event, event, sizeof(struct Athena_Event));
         }
         [lock unlock];
     }
     
     
      - (BOOL) popEvent: (struct Athena_Event *)to{
         BOOL succeeded = NO;
         [lock lock];
         
         if(events){
             struct Athena_EventList *next = events->next;
             memcpy(to, &(events->event), sizeof(struct Athena_Event));

             free(events);
             events = next;
             succeeded = YES;
         }
         
         [lock unlock];
         return succeeded;
     }


      + (void) mousePosition: (struct Athena_Event *)to whichWindow:(NSWindow *)window{
         NSPoint p = [NSEvent mouseLocation];
         NSPoint origin = [window frame].origin;
         to->x = p.x - origin.x;
         to->y = p.y - origin.y;
     }

@end

@interface Athena_Window : NSWindow
     - (BOOL)canBecomeKeyWindow;
     - (BOOL)canBecomeMainWindow;
     - (void)sendEvent:(NSEvent *)event;
     - (void)doCommandBySelector:(SEL)aSelector;
@end

@implementation Athena_Window
     - (BOOL)canBecomeKeyWindow{
        return YES;
    }

     - (BOOL)canBecomeMainWindow{
        return YES;
    }
     - (void)sendEvent:(NSEvent *)event{
     
     
    }
     - (void)doCommandBySelector:(SEL)aSelector{

    }
@end

void *Athena_Private_CreateHandle(){
    [NSApplication sharedApplication];

    return [Athena_Window alloc];
}

int Athena_Private_DestroyHandle(void *arg){
    Athena_Window *window = arg;
    [window release];
    return 0;
}

int Athena_Private_CreateWindow(void *arg, int x, int y, unsigned w, unsigned h, const char *title){
    Athena_Window * const window = arg;
    @try {
        [window initWithContentRect:NSMakeRect(x, y, w, h)
                styleMask:NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask
                backing:NSBackingStoreBuffered
                defer:NO];
    }
    @catch(NSException *e){
        fprintf(stderr, "%s\n", [[e reason] UTF8String]);
        return 1;
    }
    
    {
        NSString * const ns_string = [[NSString alloc]initWithUTF8String:title];
        [window setTitle:ns_string];
        [ns_string release];
    }

    [window setBackgroundColor:[NSColor blackColor]];
    
    {

    }
    
    return 0;
}

int Athena_Private_ShowWindow(void *arg){
    Athena_Window * const window = arg;
    [NSApp activateIgnoringOtherApps:YES];
    [window makeKeyAndOrderFront:nil];
    return 0;
}

int Athena_Private_HideWindow(void *arg){
    Athena_Window * const window = arg;
    [window orderOut:nil];
    return 0;
}

int Athena_Private_DrawImage(void *arg, int x, int y, unsigned w, unsigned h, unsigned format, const void *RGB){

    return 0;
}

int Athena_Private_DrawRect(void *arg, int x, int y, unsigned w, unsigned h, const struct Athena_Color *color){

    return 0;
}

int Athena_Private_DrawLine(void *arg, int x1, int y1, int x2, int y2, const struct Athena_Color *color){

    return 0;
}

int Athena_Private_FlipWindow(void *arg){

    return 0;
}

unsigned Athena_Private_GetEvent(void *arg, struct Athena_Event *to){

    return 0;
}

/* Athena_Common functions are common to all backends, but are private to this library.
 * These are intended to be used from the Athena_Private functions.
 * No Athena_Common function will call any Athena_Private function, to categorically avoid infinite mutual recursion.
 */
int Athena_Common_Line(void *handle, void *arg, int x1, int y1, int x2, int y2, athena_point_callback callback);
int Athena_Common_ColorToUnsignedByte(const struct Athena_Color *color, unsigned char *red, unsigned char *greeb, unsigned char *blue, unsigned char *alpha);
int Athena_Common_ColorToUnsignedShort(const struct Athena_Color *color, unsigned short *red, unsigned short *greeb, unsigned short *blue, unsigned short *alpha);
