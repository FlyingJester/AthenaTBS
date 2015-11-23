#pragma once

enum Athena_Keymap {
	scroll_up_key,
	scroll_down_key,
	scroll_right_key,
	scroll_left_key,
	ATHENA_KEYMAP_SIZE
};

struct Athena_Options{
	unsigned screen_w, screen_h;
	unsigned short keymap[ATHENA_KEYMAP_SIZE];
};

extern const char * const athena_default_settings_path;

/* Will fill to with defaults if path is either not found, is malformed, or is missing values */
int Athena_LoadOptions(const char *path, struct Athena_Options *to);
