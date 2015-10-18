#pragma once
#include "viewport.h"

struct Athena_MessageList;

struct Athena_Button{
    int x, y;
    unsigned w, h;
    const char *text;

    void *arg;
    void (*callback)(void *, struct Athena_MessageList *);
    unsigned clicked;
};

struct Athena_ButtonList{
    struct Athena_Button button;
    struct Athena_ButtonList *next;
};

struct Athena_Menu{
    struct Athena_ButtonList *buttons;
    int x, y;
    unsigned w, h;
};

int Athena_UIDrawButtons(struct Athena_ButtonList *buttons, struct Athena_Viewport *onto);
