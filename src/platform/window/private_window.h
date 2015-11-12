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
/* w and h are provided here because for some backends it is somewhat costly to query these properties,
 * while the window structure inside Athena has them cached.
 */
int Athena_Private_Update(void *handle, unsigned format, const void *RGB, unsigned w, unsigned h);

int Athena_Private_FlipWindow(void *handle);
unsigned Athena_Private_GetEvent(void *handle, struct Athena_Event *to);

int Athena_Private_GetMousePosition(void *handle, int *x, int *y);

int Athena_Private_IsKeyPressed(void *handle, unsigned key);

#ifdef __cplusplus
}
#endif
