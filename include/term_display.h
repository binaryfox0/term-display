#ifndef TERMINAL_DISPLAY_H
#define TERMINAL_DISPLAY_H

#include "term_def.h"
#include "term_texture.h"

enum display_settings_types
{
 // Boolean-related settings
 auto_resize = 0,
 // Other settings
 display_size
};

u8 display_init();
u8 display_option(enum display_settings_types type, u8 get, void* option);
// Graphics-related functions
void display_set_color(struct term_rgba color);
void display_copy_texture(
 const term_texture* texture,
 const struct term_pos pos,
 const enum texture_merge_mode mode
);
u8 display_show();
void display_free(i32 nothing);

#endif

