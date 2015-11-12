#pragma once
#include "window_defs.h"

struct Athena_Window;

struct Athena_Window *Athena_CreateWindow(unsigned w, unsigned h, const char *title);

int Athena_ShowWindow(struct Athena_Window *that);
int Athena_HideWindow(struct Athena_Window *that);
int Athena_DestroyWindow(struct Athena_Window *that);
int Athena_IsWindowShown(struct Athena_Window *that);

int Athena_WindowWidth(struct Athena_Window *that);
int Athena_WindowHeight(struct Athena_Window *that);

/* RGB must be the bpp of format (4 for 32s, 3 for 24s, etc) times Athena_WindowWidth times Athena_WindowHeight. 
 * Only full updates are performed.
 */
int Athena_Update(struct Athena_Window *that, enum Athena_ImageFormat format, const void *RGB);

/* May return 0 for no events. Non-blocking */
unsigned Athena_GetEvent(struct Athena_Window *that, struct Athena_Event *to);

int Athena_FlipWindow(struct Athena_Window *that);

/* Graphics and input utility functions.
 * All 'handle' arguments can be ignored when calling from outside the Athena window library.
 */
int Athena_Bresenhams(void *handle, void *arg, int x1, int y1, int x2, int y2, athena_point_callback callback);

/* key is the UTF8 codepoint for the key. It may or may not be case sensitive. */
int Athena_IsKeyPressed(struct Athena_Window *that, unsigned key);

int Athena_GetMousePosition(struct Athena_Window *that, int *x, int *y);
unsigned Athena_GetMouseX(struct Athena_Window *that);
unsigned Athena_GetMouseY(struct Athena_Window *that);
