#pragma once

enum Athena_EventType {
    athena_unknown_event,
    athena_quit_event,
    athena_click_event
};

enum Athena_MouseButton {
    athena_left_mouse_button,
    athena_right_mouse_button,
    athena_middle_mouse_button,
    athena_unknown_mouse_button
};

struct Athena_Event {
    enum Athena_EventType type;
    int x, y, w, h, state;
    unsigned which, id;
};

enum Athena_ImageFormat{
    RGBA32,
    ARGB32,
    RGB24
};

struct Athena_Color {
    float red, green, blue, alpha;
};

typedef void (*athena_point_callback)(void *handle_, void *arg_, int x, int y);
