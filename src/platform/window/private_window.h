#pragma once
#include "window_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Athena_Private functions are supplied by the backend.
 */

void *Athena_Private_CreateHandle();
int Athena_Private_DestroyHandle(void *);
int Athena_Private_CreateWindow(void *handle, int x, int y, unsigned w, unsigned h, const char *title);
int Athena_Private_ShowWindow(void *);
int Athena_Private_HideWindow(void *);
int Athena_Private_DrawImage(void *handle, int x, int y, unsigned w, unsigned h, unsigned format, const void *RGB);
int Athena_Private_DrawRect(void *that, int x, int y, unsigned w, unsigned h, const struct Athena_Color *color);
int Athena_Private_DrawLine(void *that, int x1, int y1, int x2, int y2, const struct Athena_Color *color);

int Athena_Private_FlipWindow(void *handle);
unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to);

/* Athena_Common functions are common to all backends, but are private to this library.
 * These are intended to be used from the Athena_Private functions.
 * No Athena_Common function will call any Athena_Private function, to categorically avoid infinite mutual recursion.
 */
int Athena_Common_Line(void *handle, void *arg, int x1, int y1, int x2, int y2, athena_point_callback callback);
int Athena_Common_ColorToUnsignedByte(const struct Athena_Color *color, unsigned char *red, unsigned char *greeb, unsigned char *blue, unsigned char *alpha);
int Athena_Common_ColorToUnsignedShort(const struct Athena_Color *color, unsigned short *red, unsigned short *greeb, unsigned short *blue, unsigned short *alpha);

int Athena_Private_IsKeyPressed(void *handle, unsigned key);

#ifdef __cplusplus
}
#endif
