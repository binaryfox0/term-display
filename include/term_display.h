#ifndef TERMINAL_DISPLAY_H
#define TERMINAL_DISPLAY_H

#include "term_def.h"
#include "term_texture.h"

typedef enum
{
 // Boolean-related settings
 auto_resize = 0,
 // Other settings
 display_size
} display_settings_types;

typedef enum { key_release, key_press, key_hold } key_state;
typedef enum {
 key_shift = 1,
 key_ctrl = 2,
 key_alt = 4
} key_mod;

u8 display_init();
u8 display_option(display_settings_types type, u8 get, void* option);
// Event-related functions
extern volatile u8 __display_is_running;
static inline u8 display_is_running() { return __display_is_running; }

typedef void (*key_callback)(int key, int mods, key_state actions);
void display_set_key_callback(key_callback callback);

// Graphics-related functions
void display_set_color(term_rgba color);
void display_copy_texture(
 const term_texture* texture,
 const term_pos pos,
 const enum texture_merge_mode mode
);
u8 display_show();
void display_free(i32 nothing);

#endif

