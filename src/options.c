#include "options.h"
#include "turbo_json_helpers.h"
#include <bufferfile/bufferfile.h>
#include <TurboJSON/parse.h>
#include <string.h>

const char * const athena_default_settings_path = "athena_settings.json";

/*
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
*/

static const unsigned short athena_default_keymap[ATHENA_KEYMAP_SIZE] = {
	'w', 's', 'd', 'a'
};

static int athena_load_default_options(struct Athena_Options *to){
	memcpy(to->keymap, athena_default_keymap, ATHENA_KEYMAP_SIZE * sizeof(unsigned short));
	to->screen_w = 400;
	to->screen_h = 300;
	return 0;
}

int Athena_LoadOptions(const char *path, struct Athena_Options *to){
	int size;
	const char *source = BufferFile(path, &size);
	
	athena_load_default_options(to);
	
	if(source){
		struct Turbo_Value val;
		int err = Turbo_Parse(&val, source, source + size);
		const struct Turbo_Value *const w = Turbo_Helper_GetConstObjectElement(&val, "screen_width"),
			*const h = Turbo_Helper_GetConstObjectElement(&val, "screen_height");
		if(err){}
		if(w){
			to->screen_w = w->value.number;
		}
		if(h){
			to->screen_h = h->value.number;
		}
		
		TurboFree(&val);
		FreeBufferFile((void *)source, size);
		return 0;
	}
	else{
		return 1;
	}
}
