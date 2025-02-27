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

typedef enum {
 term_key_space = 32,
 term_key_a = 65,
 term_key_b,
 term_key_c,
 term_key_d,
 term_key_e,
 term_key_f,
 term_key_g,
 term_key_h,
 term_key_i,
 term_key_j,
 term_key_k,
 term_key_l,
 term_key_m,
 term_key_n,
 term_key_o,
 term_key_p,
 term_key_q,
 term_key_r,
 term_key_s,
 term_key_t,
 term_key_u,
 term_key_v,
 term_key_w,
 term_key_x,
 term_key_y,
 term_key_z,
 term_key_escape = 256,
 term_key_enter = 257,
 term_key_backspace = 259,
 term_key_insert = 260,
 term_key_delete,
 term_key_right,
 term_key_left,
 term_key_down,
 term_key_up,
 term_key_page_up,
 term_key_page_down,
 term_key_home,
 term_key_end,
 term_key_f1 = 290,
 term_key_f2,
 term_key_f3,
 term_key_f4,
 term_key_f5,
 term_key_f6,
 term_key_f7,
 term_key_f8,
 term_key_f9,
 term_key_f10,
 term_key_f11,
 term_key_f12,
} key_token;

u8 display_init();
u8 display_option(display_settings_types type, u8 get, void* option);
// Event-related functions
extern volatile u8 __display_is_running;
static inline u8 display_is_running() { return __display_is_running; }

void display_poll_events();

typedef void (*key_callback_func)(int key, int mods, key_state actions);
typedef void (*resize_callback_func)(term_vec2 new_size);

void display_set_key_callback(key_callback_func callback);
void display_set_resize_callback(resize_callback_func callback);

// Graphics-related functions
void display_set_color(term_rgba color);
void display_copy_texture(
 const term_texture* texture,
 const term_pos pos,
 const enum texture_merge_mode mode
);
u8 display_show();
void display_free();

#endif

