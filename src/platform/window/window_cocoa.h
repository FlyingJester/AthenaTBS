#pragma once
#import <AppKit/NSWindow.h>
#import <AppKit/NSImageView.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSColor.h>
#import <Foundation/NSLock.h>

#include <stdint.h>

@interface Athena_Window : NSWindow
    @property(assign) NSImage *image;
    @property(assign) NSImageView *image_view;

    @property CGImageRef image_ref;
    @property CGDataProviderRef provider_ref;
    @property CGColorSpaceRef color_space_ref;
    @property struct CGDataProviderDirectCallbacks callbacks;
    @property(strong) NSMutableData *image_data;
    @property(strong) NSLock *lock;

    @property(assign) NSObject *listener;

     - (NSUInteger)getWidth;
     - (NSUInteger)getHeight;

     - (void)drawAthenaImage:(const void *)rgba position:(NSRect)at;
     
     - (void)initViews;
     
     - (BOOL)canBecomeKeyWindow;
     - (BOOL)canBecomeMainWindow;
     - (BOOL)wantsPeriodicDraggingUpdates;
     - (void)sendEvent:(NSEvent *)event;
@end

@interface Athena_ApplicationDelegate : NSResponder <NSApplicationDelegate>

     - (void) applicationDidFinishLaunching:(NSNotification *)notification;

@end

@interface Athena_WindowDelegate : NSResponder <NSWindowDelegate>

     - (void)listen:(Athena_Window*)window;

@end
