#include "term_display.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

#include "term_priv.h"
#include "term_texture_priv.h"

/* Global variable begin */
static term_texture *display = 0;
term_u8 *display_raw = 0;            // For the drawing
static term_u8 display_channel = 3;  // RGBA
static term_ivec2 size = { 0, 0 },

// Detect size change (cross-platform)
    term_size = { 0, 0 }, prev_size = { 0, 0 };

// Display clear color
term_rgba default_background = { 0, 0, 0, 255 }, 
          clear_color = {0, 0, 0, 255};

volatile term_u8 internal_failure = 0;
volatile term_bool __display_is_running = 0;
struct {
    term_i32 x_start, x_end, x_inc;
    term_i32 y_start, y_end, y_inc;
} display_prop = { 0, 0, 1, 0, 0, 1 };

term_f32 *depth_buffer = 0;

term_u8 numeric_options[__td_opt_numeric_end__] = {
    [td_opt_auto_resize] = 0,
    [td_opt_pixel_width] = 2,
    [td_opt_pixel_height] = 1,
    [td_opt_display_type] = display_truecolor,
    [td_opt_display_rotate] = 0,
    [td_opt_depth_buffer] = 0
};

/* Global variable end */

TD_INLINE void clear_screen()
{
    printf("\x1b[0m"            // Reset colors mode
           "\x1b[3J"            // Clear saved line (scrollbuffer)
           "\x1b[H"             // To position 0,0
           "\x1b[2J"            // Clear entire screen
        );
}

TD_INLINE void query_default_background()
{
    static const char *request = "\x1b]11;?\x1b\\";
    _pwrite(STDOUT_FILENO, request, strlen(request));
    if (!timeout(1))
        return;
    char buffer[32] = { 0 };
    if (_pread(STDIN_FILENO, buffer, 32) == -1)
        return;
    char r[5] = { 0 }, b[5] = { 0 }, g[5] = { 0 };
    if (sscanf(buffer, "\x1B]11;rgb:%4[^/]/%4[^/]/%4[^;]", r, g, b) != 3)
        return;
    default_background =
        rgba_init((term_u8)(strtol(r, 0, 16) / 257),
                  (term_u8)(strtol(g, 0, 16) / 257), (term_u8)(strtol(b, 0, 16) / 257), 255);
}

/* Utils function end */

const char *td_copyright_notice()
{
    return
        "--------Terminal renderer library\n--------"
        "Copyright (c) 2025 binaryfox0 (Duy Pham Duc)\n"
        "License under the MIT License (see LICENSE file)\n\n"
        "Extremely Fast Line Algorithm Var D (Addition Fixed Point)\n"
        "Copyright 2001, By Po-Han Lin\n"
        "Freely usable in non-commercial applications as long as \n"
        "credits to Po-Han Lin and a link to http://www.edepot.com \n"
        "is provided in source code and can be seen in compiled executable.\n"
        "Commercial applications please inquire about licensing the algorithms.";
}

TD_INLINE void reset_depth_buffer()
{
    if(!depth_buffer) return;
    term_u64 buf_size = calculate_size(size.x, size.y, sizeof(term_f32));
    depth_buffer[0] = FLT_MAX;
    char* ptr = (char*)depth_buffer;
    term_u64 filled = sizeof(term_f32);
    while (filled * 2 < buf_size) {
        memcpy(&ptr[filled], ptr, filled);
        filled *= 2;
    }
    memcpy(&ptr[filled], ptr, buf_size - filled);
}

void resize_depth_buffer()
{
    if (!numeric_options[td_opt_depth_buffer])
        return;
    term_u64 buf_size = calculate_size(size.x, size.y, sizeof(term_f32));
    term_f32 *tmp =
        (term_f32 *) realloc(depth_buffer, buf_size);
    if (!tmp) {
        internal_failure = 1;
        return;
    }
    depth_buffer = tmp;
    reset_depth_buffer();
}

void resize_display()
{
    size =
        ivec2_init(term_size.x / numeric_options[td_opt_pixel_width],
                   term_size.y / numeric_options[td_opt_pixel_height]);
    if ((internal_failure = texture_resize_internal(display, size)))
        return;                 // Uhhhh, how to continue processing without the display
    display_raw = texture_get_location(ivec2_init(0, 0), display);
    if (display_prop.y_inc < 0)
        display_prop.y_start = size.y - 1;
    else
        display_prop.y_end = size.y - 1;
    if (display_prop.x_inc < 0)
        display_prop.x_start = size.x - 1;
    else
        display_prop.x_end = size.x - 1;
    texture_fill(display, clear_color);
    clear_screen();
    resize_depth_buffer();
}

#if defined(TERMINAL_WINDOWS)
BOOL stop_display(DWORD ctrl_type)
{
    switch(ctrl_type)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
            __display_is_running = term_false;
            return FALSE;
        default:
            return FALSE;
    }
    return FALSE;
}
#else
void stop_display(int signal)
{
    (void) signal;
    __display_is_running = 0;
}
#endif

void reload_display()
{
    if (display) {
        texture_free(display);
        display = 0;            // Prevent free on freed memory section
    }
    if (!
        (display =
         texture_create(0, display_channel, ivec2_init(1, 1), 0, 0))) {
        internal_failure = 1;
        return;
    }
    resize_display();
}


term_bool td_init()
{
    if (!_pisatty(STDOUT_FILENO))
        return 1;
    if (setup_env(stop_display))
        return 1;
    term_size = prev_size = query_terminal_size();      // Disable calling callback on the first time
    query_default_background();
    clear_color = default_background;   // Look like the background
    _pwrite(STDOUT_FILENO, "\x1b[?25l\x1b[?1049h", 15);     // Hide cursor and enable buffer
    reload_display();
    if (internal_failure)
        return 1;
    __display_is_running = 1;
    return 0;
}

TD_INLINE term_bool opt_get(term_bool get, void *option, void *value, size_t s) {
    if (get) {
        memcpy(option, value, s);
        return 1;
    }
    return 0;
}

#define OPT_GET(type, value) \
    if (opt_get(get, option, &(value), sizeof(type))) return 0;

#define OPT_SET(type, target) \
    (target) = *(type*)option

term_bool td_option(td_settings_t type, term_bool get, void *option) {
    switch (type) {
    case td_opt_auto_resize: {
        OPT_GET(term_u8, numeric_options[type]);
        OPT_SET(term_u8, numeric_options[type]);
        break;
    }

    case td_opt_pixel_width:
    case td_opt_pixel_height: {
        OPT_GET(term_u8, numeric_options[type]);
        OPT_SET(term_u8, numeric_options[type]);
        resize_display();
        break;
    }

    case td_opt_display_size: {
        OPT_GET(term_ivec2, size);
        OPT_SET(term_ivec2, size);
        if ((internal_failure = texture_resize_internal(display, size)))
            return 1;
        texture_fill(display, clear_color);
        clear_screen();
        break;
    }

    case td_opt_display_type: {
        OPT_GET(display_types, numeric_options[type]);
        switch(OPT_SET(display_types, numeric_options[type])) {
        case display_grayscale_24:
        case display_grayscale_256: display_channel = 1; break;
        case display_truecolor_216:
        case display_truecolor: display_channel = 3; break;
        default:
            return 1;
        }
        reload_display();
        break;
    }

    case td_opt_display_rotate: {
        OPT_GET(term_u8, numeric_options[type]);
        switch((numeric_options[type] = *(term_u8 *) option % 4)) {
        case 0:
            display_prop.y_inc = 1;
            display_prop.y_start = 0;
            display_prop.y_end = size.y - 1;
            break;
        case 1:
            display_prop.y_inc = -1;
            display_prop.y_start = size.y - 1;
            display_prop.y_end = -1;
            display_prop.x_inc = -1;
            display_prop.x_start = size.x - 1;
            break;
        default:
            return 1;
        }
        break;
    }

    case td_opt_depth_buffer: {
        OPT_GET(term_u8, numeric_options[type]);
        if(OPT_SET(term_u8, numeric_options[type])){
            resize_depth_buffer();
        } else {
            if(depth_buffer) free(depth_buffer);
            depth_buffer = NULL;
        }
        break;
    }

    case td_opt_disable_stop_sig:
    {
        term_bool tmp = *(term_bool*)option;
        if(tmp) {
            if(disable_stop_sig()) return 1;
        } else {
            if(enable_stop_sig()) return 1;
        }
        numeric_options[type] = tmp;
        break;
    }

    case td_opt_shift_translate: {
        OPT_GET(term_bool, shift_translate);
        OPT_SET(term_bool, shift_translate);
        break;
    }

    default:
        return 1;
    }

    return 0;
}


// Default callback
key_callback_func private_key_callback = 0;
resize_callback_func private_resize_callback = 0;
void td_poll_events()
{
    kbpoll_events(private_key_callback);
    if (!ivec2_equal((term_size = query_terminal_size()), prev_size)) {
        if (numeric_options[td_opt_auto_resize]) {
            resize_display();
            if(private_resize_callback) private_resize_callback(size);
        } else
            clear_screen();
        prev_size = term_size;
    }
}

void td_set_key_callback(key_callback_func callback)
{
    private_key_callback = callback;
}

void td_set_resize_callback(resize_callback_func callback)
{
    private_resize_callback = callback;
}

void td_set_color(term_rgba color)
{
    clear_color = pixel_blend(default_background, color);
    texture_fill(display, clear_color);
    reset_depth_buffer();
}

void td_copy_texture(const term_texture *texture,
                          const term_vec2 pos,
                          const enum texture_merge_mode mode)
{
    term_ivec2 display_pos = ndc_to_pos(pos, size);
    texture_merge(display, texture, display_pos, mode, 0);
}

#define VERTEX_BUF_SIZ 3

static term_i32 vertex_count = 0;
static const term_f32 vertex_default[4] = {0.0f, 0.0f, 0.0f, 1.0f};
static term_f32 vertex_buffer[4*VERTEX_BUF_SIZ] = {0};
void td_render_flush()
{
    for(term_i32 i = 0; i < VERTEX_BUF_SIZ; i++)
        memcpy(vertex_buffer, vertex_default, VERTEX_BUF_SIZ * sizeof(term_f32));
}

void td_render_add(const term_f32* vertices, term_i32 component)
{
    memcpy(&vertex_buffer[vertex_count * 4], vertices, (size_t)component * sizeof(term_f32));
    vertex_count++;
    if(vertex_count == 3)
    {
        // Homogeneous position conversion
        term_ivec2 p1 = ndc_to_pos(vec2_init(vertex_buffer[0] / vertex_buffer[3], vertex_buffer[1] / vertex_buffer[3]), size);
        term_ivec2 p2 = ndc_to_pos(vec2_init(vertex_buffer[4] / vertex_buffer[7], vertex_buffer[5] / vertex_buffer[7]), size);
        term_ivec2 p3 = ndc_to_pos(vec2_init(vertex_buffer[8] / vertex_buffer[11], vertex_buffer[9] / vertex_buffer[11]), size);
        ptexture_draw_triangle(display_raw, size, display_channel,
            vertex_init(p1, vertex_buffer[2],  rgba_init(255,0,  0,   255)),
            vertex_init(p2, vertex_buffer[6],  rgba_init(0,  255,0,   255)),
            vertex_init(p3, vertex_buffer[10], rgba_init(0,  0,  255, 255)),
            depth_buffer);
        vertex_count = 0;
        td_render_flush();
    }
}

void td_draw_line(const term_vec2 p1, const term_vec2 p2,
                       const term_rgba color)
{
    ptexture_draw_line(display_raw, size, display_channel,
                       ndc_to_pos(p1, size),
                       ndc_to_pos(p2, size), 
                       vec2_init(0.0f, 0.0f),color, 0);
}

TD_INLINE term_u8 rgb_to_216(const term_u8 *c)
{
    return (term_u8)(16 + (c[0] / 51 * 36) + (c[1] / 51 * 6) + (c[2] / 51));
}

TD_INLINE void display_cell(term_u8 *c)
{
    if (display_channel == 1) {
        if (numeric_options[td_opt_display_type] ==
            display_grayscale_256)
            printf("\x1b[48;2;%d;%d;%dm", c[0], c[0], c[0]);
        else if (numeric_options[td_opt_display_type] ==
                 display_grayscale_24)
            printf("\x1b[48;5;%dm", 232 + ((c[0] * 24) >> 8));
    } else if (display_channel == 3) {
        if (numeric_options[td_opt_display_type] == display_truecolor)
            printf("\x1b[48;2;%d;%d;%dm", c[0], c[1], c[2]);
        if (numeric_options[td_opt_display_type] ==
            display_truecolor_216)
            printf("\x1b[48;5;%dm", rgb_to_216(c));
    }
}

// ANSI escape sequence https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
term_bool td_show()
{
    if (internal_failure)
        return 1;
    
    printf("\x1b[H");
    term_u8 *ptr = display_raw;
    static term_u8 prev[3] = { 0 };
    for (term_i32 row = display_prop.y_start; row != display_prop.y_end;
         row += display_prop.y_inc) {
        for (term_u8 i = 0; i < numeric_options[td_opt_pixel_height]; i++) {
            term_u8 *ref = &ptr[(row * size.x + display_prop.x_start)* display_channel];
            for (term_u16 col = 0; col < size.x; col++, ref += display_prop.x_inc * display_channel) {
                if (memcmp(prev, ref, display_channel) != 0) {
                    display_cell(ref);
                    memcpy(prev, ref, display_channel);
                }
                printf("%*s", numeric_options[td_opt_pixel_width], "");
            }
            printf("\x1b[1E");
        }
    }
    return 0;
}

void td_free()
{
    // Show the cursor again
    // Reset color / graphics mode
    _pwrite(STDOUT_FILENO, "\x1b[?25h\x1b[0m\x1b[?1049l", 19);
    fflush(stdout);             // Flush remaining data
    restore_env();
    texture_free(display);
}
