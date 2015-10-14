#include "private_window.h"
#import <Cocoa/Cocoa.h>
#import "window_cocoa.h"

@implementation Athena_ApplicationDelegate
     - (void) applicationDidFinishLaunching:(NSNotification *)notification{
        
    }
     - (void)userTapQuitMenu:(id)sender{
        [[NSApplication sharedApplication] terminate:self];
    }
@end

@implementation Athena_Window
     - (NSUInteger)getWidth{
        return  [[self contentView] frame].size.width;
    }
     - (NSUInteger)getHeight{
        return  [[self contentView] frame].size.height;
    }

     - (BOOL)canBecomeKeyWindow{
        return YES;
    }

     - (BOOL)canBecomeMainWindow{
        return YES;
    }

     - (BOOL)wantsPeriodicDraggingUpdates{
        return NO;
    }
    
     - (void)sendEvent:(NSEvent *)event{
        [super sendEvent:event];
    }

     - (void)drawAthenaImage:(const void *)rgba position:(NSRect)at{
        
    }

     - (void)initViews{
        const size_t backing_size = [self getWidth] * [self getHeight] << 2;
        uint8_t *data_backing = malloc(backing_size);
        
        _image_data = [[NSMutableData alloc] initWithBytesNoCopy:data_backing length:backing_size];
        
        _lock = [[NSLock alloc] init];
        _color_space_ref = CGColorSpaceCreateDeviceRGB();
        _provider_ref = CGDataProviderCreateWithCFData((__bridge CFDataRef)_image_data);
        
        _image_ref = CGImageCreate([self getWidth], [self getHeight],
            8,   /* bits per channel */
            32,  /* bits per pixel */
            [self getWidth] << 2, /* Pitch */
            _color_space_ref,
            kCGBitmapByteOrder32Little,
            _provider_ref,
            NULL,
            YES,
            kCGRenderingIntentDefault);
        _image = [[NSImage alloc] initWithCGImage:_image_ref size:NSZeroSize];
            
        _image_view = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 0, [self getWidth], [self getHeight])];
        [_image_view setImage:_image];
        [[self contentView] addSubview: _image_view];
    }
@end

@implementation Athena_WindowDelegate
     - (void)listen:(Athena_Window*)window{
        [window setDelegate:self];
        [window setNextResponder:self];
        [window setListener:self];
    }
@end

void *Athena_Private_CreateHandle(){
    if(NSApp == nil){
        [NSApplication sharedApplication];
        [NSApp setMainMenu:[[NSMenu alloc] init]];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp finishLaunching];
    }
    return [Athena_Window alloc];
}

int Athena_Private_DestroyHandle(void *handle){
    [(Athena_Window *)handle release];
    return 0;
}

int Athena_Private_CreateWindow(void *handle, int x, int y, unsigned w, unsigned h, const char *title){
    Athena_Window *const window = handle;
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
    
    [[[Athena_WindowDelegate alloc] init] listen:window];
    [window initViews];
    
    return 0;
}

int Athena_Private_ShowWindow(void *handle){
    puts("FRONT");
    [(Athena_Window *)handle makeKeyAndOrderFront:nil];
    return 0;
}

int Athena_Private_HideWindow(void *handle){
    [(Athena_Window *)handle orderOut:nil];
    return 0;
}

int Athena_Private_DrawImage(void *handle, int x, int y, unsigned w, unsigned h, unsigned format, const void *RGB){
    
    return 0;
}

int Athena_Private_FlipWindow(void *handle){
    
    return 0;
}

unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to){
    NSEvent *event = nil;
    do{
        NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
        if([event window])
            printf("Window %p (%p)\n", [event window], handle);
        if([event window] == handle){
            puts("Event captured...");
            /* Set Type */
            switch([event type]){
                case NSLeftMouseDown:
                case NSRightMouseDown:
                case NSOtherMouseDown:
                    to->type = athena_click_event;
                    break;
                default:
                    to->type = athena_unknown_event;
            }

            if([event type] == NSLeftMouseDown)
                to->which = athena_left_mouse_button;
            else if([event type] == NSRightMouseDown)
                to->which = athena_right_mouse_button;
            else if([event type] == NSRightMouseDown)
                to->which = athena_middle_mouse_button;
                /*
                    ...
                */
        }
        else{
            [NSApp sendEvent:event];
        }
    }while(event != nil);
    
    return 0;
}

/* Athena_Common functions are common to all backends, but are private to this library.
 * These are intended to be used from the Athena_Private functions.
 * No Athena_Common function will call any Athena_Private function, to categorically avoid infinite mutual recursion.
 */
int Athena_Common_Line(void *handle, void *arg, int x1, int y1, int x2, int y2, athena_point_callback callback);
int Athena_Common_ColorToUnsignedByte(const struct Athena_Color *color, unsigned char *red, unsigned char *greeb, unsigned char *blue, unsigned char *alpha);
int Athena_Common_ColorToUnsignedShort(const struct Athena_Color *color, unsigned short *red, unsigned short *greeb, unsigned short *blue, unsigned short *alpha);
