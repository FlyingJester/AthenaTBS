#pragma once
#include "unit.h"
#include "field.h"
#include "image.h"
#include "button.h"
#include "menu.h"
#include "pathfind.h"
#include "options.h"
#include "audio/audio.h"
#include "monitor/monitor.h"
#include "window/window.h"
#include <TurboJSON/value.h>

/* Used to parse a message passed in the engine format. 
 * The message can use either our own athena protocol,
 * or http to encapsulate the JSON data. 
 * 
 * The read_function is expected to return `len` bytes of data past the last read called with `arg`.
 * See Athena_MemoryReadFunction for an example if you really need one.
 * free_function may be NULL, to indicate we don't need to free the result.
 */
int Athena_ParseEngineMessage(struct Athena_MessageList *to, const char *(*read_function)(void *arg, uint32_t len), void(*free_function)(void *arg, const char *z), void *arg);

struct Athena_MessageMemoryBuffer{
    const char *data;
    uint64_t len, at;
};

/* For use with Athena_ParseEngineMessage. `arg` must be a Athena_MessageMemoryBuffer. */
const char *Athena_MemoryReadFunction(void *arg, uint32_t len);

/* Supplied to call free() on z. Should be enough for most read_functions. */
void Athena_FreeReadWrapper(void *, const char *z);

struct Athena_MessageList{
    /* This is the owning pointer to this text */
    char *msg_text;
    struct Turbo_Value value;
    struct Athena_MessageList *next;
};

/* Event queue and locking monitor to get events on. `status' will be made -1 on exit or error. */
struct Athena_EventData{
    struct Athena_MessageList *msg;
};

struct Athena_SelectingPosition{
    struct Athena_Unit *unit;

    /* In tile coordinates */
    int x, y;
};

struct Athena_BuildPosition{
    struct Athena_Class *clazz;

    /* In tile coordinates */
    int x, y;
};

/* UI and graphics data. This must only ever be touched by the UI thread. */
struct Athena_UI{
    int camera_x, camera_y;
    struct{
        int64_t time;
        unsigned event_x, event_y;
        enum {ui_cam_none, ui_cam_mouse, ui_cam_key} type;
    } last_camera_motion;

    struct Athena_Image framebuffer;
    struct Athena_Window *window;
    struct Athena_ButtonList *buttons;
    struct Athena_Sound *click_sound;

    /*
     * When callback is not null, we are selecting a position on the map.
     * The result is stored as Athena_SelectingPosition appended to the end
     * of the arglist that was given.
     * The selection_arg is always freed _after_ the callback is used, and
     * the callback is nulled. We do not keep trying to get a selection if
     * there was a failure.
     */
    struct Athena_ButtonArgList *selection_arg;
    void (*selection_callback)(struct Athena_ButtonArgList *, struct Athena_MessageList *);
    
    /* This callback is nulled at the same time selection_callback is.
     * it populates a list of spaces that are valid targets.
     * The arg is freed at the same time that selection_arg is.
     */
    struct Athena_ButtonArgList *positions_arg;
    struct Athena_PositionList *(*positions_callback)(struct Athena_ButtonArgList *);
    
    /* This is intended for drawing an overlay. As such, it is called after ALL
     * other drawing code. Additionally, while this value is set, all other event 
     * callbacks are disabled.
     * If this callback returns 0, then it will be unset and all associated values will
     * be freed, ending this overlay.
     */
    struct Athena_ButtonArgList *overlay_arg;
    unsigned (*overlay_event_callback)(struct Athena_ButtonArgList *, const struct Athena_Event *event, struct Athena_MessageList *);
    void (*overlay_draw_callback)(const struct Athena_ButtonArgList *, struct Athena_Image *framebuffer);

    /* Yes, we have only one menu open at a time. We may want to roll this 
     * into the overlay system at some point (as in an overlay callback that 
     * simply draws a menu).*/
    struct Athena_Menu *menu;
};

struct Athena_GameState{
    struct Athena_Monitor *monitor;
    struct Athena_Field *field;
    struct Athena_EventData event;
    struct Athena_Player *players;
    unsigned whose_turn, num_players;
    struct Athena_UI ui;
    int status;
};

typedef struct Athena_Player *(*athena_field_condition)(const struct Athena_Field *field, unsigned num_players);
struct Athena_Player *Athena_ConquestCondition(const struct Athena_Field *field, unsigned num_players);

int Athena_Game(struct Athena_Field *field, unsigned num_players, struct Athena_Player *players, struct Athena_Window *window, const struct Athena_Options *options, athena_field_condition win_condition);
void Athena_AppendMessageList(struct Athena_MessageList **to, struct Athena_MessageList *next);

/* Message Creation facilities */
char *Athena_CreateEndTurnMessage(int *size);
char *Athena_CreateMovementMessage(int *size, struct Athena_Unit *that, int to_x, int to_y);
char *Athena_CreateBuildMessage(int *size, struct Athena_Unit *that, struct Athena_Class *clazz, int to_x, int to_y);
char *Athena_CreateAttackMessage(int *size, struct Athena_Unit *that, int to_x, int to_y);

int Athena_GameTest();
