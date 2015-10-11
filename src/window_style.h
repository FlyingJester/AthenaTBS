#pragma once
#include "viewport.h"
#include <stdint.h>

typedef void (*athena_window_style_callback)(struct Athena_Viewport *onto, void *arg, uint32_t mask);

void Athena_DefaultWindowStyleCallback(struct Athena_Viewport *onto, void *arg, uint32_t mask);

struct Athena_WindowStyle{
    void *arg;
    uint32_t mask;
    athena_window_style_callback callback;
};

void Athena_DefaultWindowStyle(struct Athena_WindowStyle *into);
void Athena_DrawWindowStyle(struct Athena_WindowStyle *style, struct Athena_Viewport *onto);
