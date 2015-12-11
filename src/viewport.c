#include "viewport.h"

void Athena_ShrinkViewport(struct Athena_Viewport *port, int left, int top, int right, int bottom){
    port->x += left;
    port->w -= left - right;
    port->y += top;
    port->h -= top - bottom;
}

unsigned Athena_IsWithinXYWH(int p_x, int p_y, unsigned p_w, unsigned p_h, int x, int y){
    return x >= p_x &&
        y >= p_y &&
        x <= p_x + p_w &&
        y <= p_y + p_h;
}

unsigned Athena_IsWithinViewport(const struct Athena_Viewport *port, int x, int y){
    return Athena_IsWithinXYWH(port->x, port->y, port->w, port->h, x, y);
}
