#pragma once
#include "game.h"

/* In reality this must NOT be run on a different thread, since basically all
 * UI setups can't handle that, or at least require much more complex setup for
 * multi-threaded access. Instead, this is simply the function to call for each
 * frame of the UI. We use this configuration to ease use for delegates or handler
 * code on certain UI setups (Android and Cocoa being good examples).
 *
 * We use the same API as the server thread for consistency, and so that we could,
 * if we wanted, run this on a different thread. But don't do that unless you really
 * know what you are doing!
 * 
 * 
 */
void Athena_UIThreadWrapper(void *that);
int Athena_UIThread(struct Athena_GameState *that);

/* This should be called once per frame. Athena_UIThread[Wrapper] does this, sort of. */
int Athena_UIThreadFrame(struct Athena_GameState *that);

void Athena_UIInit(struct Athena_GameState *state);

void Athena_CancelMenuCallback(void *arg, struct Athena_MessageList *messages);
extern const struct Athena_Button athena_cancel_button;
