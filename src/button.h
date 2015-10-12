#pragma once

struct Athena_Button{
    int x, y;
    unsigned w, h;

    void *arg;
    void (*callback)(void *);
};


struct Athena_ButtonList{
    struct Athena_Button button;
    struct Athena_ButtonList *next;
};
