#pragma once
#include "window_defs.h"

struct Athena_Window;

struct Athena_Window *Athena_CreateWindow(unsigned w, unsigned h, const char *title);

int Athena_ShowWindow(struct Athena_Window *that);
int Athena_HideWindow(struct Athena_Window *that);
int Athena_DestroyWindow(struct Athena_Window *that);
int Athena_IsWindowShown(struct Athena_Window *that);

int Athena_DrawImage(struct Athena_Window *that, int x, int y, unsigned w, unsigned h, enum Athena_ImageFormat format, const void *RGB);
int Athena_DrawRect(struct Athena_Window *that, int x, int y, unsigned w, unsigned h, const struct Athena_Color *color);
int Athena_DrawLine(struct Athena_Window *that, int x1, int y1, int x2, int y2, const struct Athena_Color *color);

unsigned Athena_GetEvent(struct Athena_Window *that, struct Athena_Event *to);

int Athena_FlipWindow(struct Athena_Window *that);

/* Graphics and input utility functions.
 * All 'handle' arguments can be ignored when calling from outside the Athena window library.
 */
int Athena_Bresenhams(void *handle, void *arg, int x1, int y1, int x2, int y2, athena_point_callback callback);
