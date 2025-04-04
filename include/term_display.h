#ifndef TERMINAL_DISPLAY_H
#define TERMINAL_DISPLAY_H

#include "term_def.h"
#include "term_texture.h"

typedef enum {
    // Numeric settings
    td_opt_auto_resize = 0,
    td_opt_pixel_width,
    td_opt_pixel_height,
    td_opt_display_type,
    td_opt_display_rotate,
    td_opt_depth_buffer,
    td_opt_shift_translate,
    __td_opt_numeric_end__,
    // Other settings
    td_opt_display_size
} td_settings_t;
typedef enum {
    display_grayscale_24,
    display_grayscale_256,
    display_truecolor_216,
    display_truecolor
} display_types;

typedef enum { key_release, key_press, key_hold } td_key_state_t;
typedef enum {
    td_key_shift = 1,
    td_key_ctrl = 2,
    td_key_alt = 4
} td_key_mod_t;

typedef enum {
    td_key_space = 32,
    td_key_astrophe = 39,
    td_key_minus = 45,
    td_key_0 = 48,
    td_key_1,
    td_key_2,
    td_key_3,
    td_key_4,
    td_key_5,
    td_key_6,
    td_key_7,
    td_key_8,
    td_key_9,
    td_key_semicolon = 59,
    td_key_equal = 61,
    td_key_a = 65,
    td_key_b,
    td_key_c,
    td_key_d,
    td_key_e,
    td_key_f,
    td_key_g,
    td_key_h,
    td_key_i,
    td_key_j,
    td_key_k,
    td_key_l,
    td_key_m,
    td_key_n,
    td_key_o,
    td_key_p,
    td_key_q,
    td_key_r,
    td_key_s,
    td_key_t,
    td_key_u,
    td_key_v,
    td_key_w,
    td_key_x,
    td_key_y,
    td_key_z,
    td_key_left_bracket = 91, /*[ */
    td_key_backslash,         /*\ */
    td_key_right_bracket,     /*] */
    td_key_grave_accent = 96, /*` */
    td_key_escape = 256,
    td_key_enter,
    td_key_tab,
    td_key_backspace,
    td_key_insert,
    td_key_delete,
    td_key_right,
    td_key_left,
    td_key_down,
    td_key_up,
    td_key_page_up,
    td_key_page_down,
    td_key_home,
    td_key_end,
    td_key_f1 = 290,
    td_key_f2,
    td_key_f3,
    td_key_f4,
    td_key_f5,
    td_key_f6,
    td_key_f7,
    td_key_f8,
    td_key_f9,
    td_key_f10,
    td_key_f11,
    td_key_f12,
} td_key_token_t;

char *td_copyright_notice();

term_bool td_init();
term_bool td_option(td_settings_t type, term_bool get, void *option);
// Event-related functions
extern volatile term_bool __display_is_running;
static inline term_bool td_is_running()
{
    return __display_is_running;
}

void td_poll_events();

typedef void (*key_callback_func)(int key, int mods, td_key_state_t actions);
typedef void (*resize_callback_func)(term_ivec2 new_size);

void td_set_key_callback(key_callback_func callback);
void td_set_resize_callback(resize_callback_func callback);

// Graphics-related functions
void td_set_color(term_rgba color);
void td_copy_texture(const term_texture * texture,
                          const term_vec2 pos,
                          const enum texture_merge_mode mode);

void td_draw_line(term_vec2 p1, term_vec2 p2, term_rgba color);

typedef enum {
    td_vertex_position = 1,
    td_vertex_texture = 2
} td_vertex_component;

void td_render_flush();
void td_render_add(const term_f32* vertices, term_i32 component);

term_bool td_show();
void td_free();

#endif
