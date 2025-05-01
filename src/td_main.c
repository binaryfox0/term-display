/*
MIT License

Copyright (c) 2025 binaryfox0 (Duy Pham Duc)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "td_main.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

#include "td_priv.h"
#include "td_rasterizer.h"

/* Global variable begin */
static term_texture *display = 0;
static td_u8 *display_raw = 0;            // For the drawing
static td_u8 display_channel = 3;  // RGBA
static term_ivec2 size = { 0, 0 },
    term_size = { 0, 0 }, prev_size = { 0, 0 };

// Display clear color
term_rgba default_background = { .r = 0, .g = 0, .b = 0, .a = 255 }, 
          clear_color = { .r = 0, .g = 0, .b = 0, .a = 255};

volatile td_u8 internal_failure = 0;
volatile term_bool __display_is_running = term_false,
    td_initialized = term_false;
struct {
    td_i32 x_start, x_end, x_inc;
    td_i32 y_start, y_end, y_inc;
} display_prop = { 0, 0, 1, 0, 0, 1 };

td_f32 *depth_buffer = 0;

td_i32 numeric_options[__td_opt_numeric_end__] = {
    [td_opt_auto_resize] = 0,
    [td_opt_pixel_width] = 2,
    [td_opt_pixel_height] = 1,
    [td_opt_display_type] = td_display_truecolor,
    [td_opt_display_rotate] = 0,
    [td_opt_depth_buffer] = 0,
    [td_opt_shift_translate] = 1,
    [td_opt_enable_downscaling] = 0,
    [td_opt_original_scailing_ratio] = 1
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
        rgba_init((td_u8)(strtol(r, 0, 16) / 257),
                  (td_u8)(strtol(g, 0, 16) / 257), (td_u8)(strtol(b, 0, 16) / 257), 255);
}

TD_INLINE void calculate_display_size() {
    size =
        ivec2_init(term_size.x / numeric_options[td_opt_pixel_width],
                   term_size.y / numeric_options[td_opt_pixel_height]);
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
    td_u64 buf_size = calculate_size(size.x, size.y, sizeof(td_f32));
    depth_buffer[0] = FLT_MAX;
    fill_buffer(depth_buffer + 1, depth_buffer, buf_size - sizeof(float), sizeof(float));
}

void resize_depth_buffer()
{
    if (!numeric_options[td_opt_depth_buffer])
        return;
    td_u64 buf_size = calculate_size(size.x, size.y, sizeof(td_f32));
    td_f32 *tmp =
        (td_f32 *) realloc(depth_buffer, buf_size);
    if (!tmp) {
        internal_failure = 1;
        return;
    }
    depth_buffer = tmp;
    reset_depth_buffer();
}

void resize_display()
{
    if ((internal_failure = tdt_resize_internal(display, size)))
        return;                 // Uhhhh, how to continue processing without the display
    display_raw = tdt_get_location(ivec2_init(0, 0), display);
    if (display_prop.y_inc < 0)
        display_prop.y_start = size.y - 1;
    else
        display_prop.y_end = size.y - 1;
    if (display_prop.x_inc < 0)
        display_prop.x_start = size.x - 1;
    else
        display_prop.x_end = size.x - 1;
    tdt_fill(display, clear_color);
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
    __display_is_running = term_false;
}
#endif

term_bool td_init()
{
    if(td_initialized) return term_true;
    if (!_pisatty(STDOUT_FILENO))
        return 1;
    if (setup_env(stop_display))
        return 1;
    
    term_size = prev_size = query_terminal_size();      // Disable calling callback on the first time
    query_default_background();
    clear_color = default_background;   // Look like the background
    _pwrite(STDOUT_FILENO, "\x1b[?25l\x1b[?1049h", 15);     // Hide cursor and enable buffer

    if (!
        (display =
        tdt_create(0, display_channel, ivec2_init(1, 1), 0, 0))) {
        internal_failure = 1;
        return 1;
    }
    if(!size.x || !size.y)
        calculate_display_size();
    resize_display();

    if (internal_failure)
        return 1;
    __display_is_running = term_true;
    td_initialized = term_true;
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
        OPT_GET(td_u8, numeric_options[type]);
        OPT_SET(td_u8, numeric_options[type]);
        break;
    }

    case td_opt_pixel_width:
    case td_opt_pixel_height: {
        OPT_GET(td_u8, numeric_options[type]);
        td_u8 tmp = *(td_u8*)option;
        if(!tmp) return term_true;
        numeric_options[type] = tmp;
        calculate_display_size();
        resize_display();
        break;
    }

    case td_opt_display_size: {
        OPT_GET(term_ivec2, size);
        term_ivec2 tmp = *(term_ivec2*)option;
        if(!tmp.x || !tmp.y) return term_true;
        size = tmp;
        if(!td_initialized) return term_false;
        if ((internal_failure = tdt_resize_internal(display, size)))
            return 1;
        tdt_fill(display, clear_color);
        clear_screen();
        break;
    }

    case td_opt_display_type: {
        OPT_GET(td_display_types, numeric_options[type]);
        td_display_types tmp = *(td_display_types*)option;
        switch(tmp) {
        case td_display_grayscale_24:
        case td_display_grayscale_256: display_channel = 1; break;
        case td_display_truecolor_216:
        case td_display_truecolor: display_channel = 3; break;
        default:
            return 1;
        }
        numeric_options[type] = tmp;
        if(td_initialized) tdt_set_channel(display, display_channel);
        break;
    }

    case td_opt_display_rotate: {
        OPT_GET(td_u8, numeric_options[type]);
        switch((numeric_options[type] = *(td_u8 *) option % 4)) {
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
        OPT_GET(td_u8, numeric_options[type]);
        if((OPT_SET(td_u8, numeric_options[type]))){
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
        if((tmp ? disable_stop_sig() : enable_stop_sig()))
            return 1;
        numeric_options[type] = tmp;
        break;
    }

    case td_opt_shift_translate: {
        OPT_GET(term_bool, shift_translate);
        OPT_SET(term_bool, shift_translate);
        break;
    }

    case td_opt_enable_downscaling: {
        OPT_GET(term_bool, numeric_options[type]);
        OPT_SET(term_bool, numeric_options[type]);
        break;
    }

    case td_opt_original_scailing_ratio: {
        OPT_GET(term_bool, numeric_options[type]);
        OPT_SET(term_bool, numeric_options[type]);
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
            calculate_display_size();
            resize_display();
            if(private_resize_callback) private_resize_callback(size);
        } else
            clear_screen();
        prev_size = term_size;
    }
}

void td_set_key_callback(key_callback_func callback) {
    private_key_callback = callback;
}

void td_set_resize_callback(resize_callback_func callback) {
    private_resize_callback = callback;
}

void td_set_color(term_rgba color)
{
    clear_color = pixel_blend(default_background, color);
    tdt_fill(display, clear_color);
    reset_depth_buffer();
}

void td_copy_texture(const term_texture *texture,
                          const term_vec2 pos,
                          const enum tdt_merge_mode mode)
{
    term_ivec2 display_pos = ndc_to_pos(pos, size);
    tdt_merge(display, texture, display_pos, mode, 0);
}

#define VERTEX_BUF_SIZ 3

static td_i32 vertex_count = 0;
static const td_f32 vertex_default[4] = {0.0f, 0.0f, 0.0f, 1.0f};
static td_f32 vertex_buffer[4*VERTEX_BUF_SIZ] = {0};
void td_render_flush()
{
    for(td_i32 i = 0; i < VERTEX_BUF_SIZ; i++)
        memcpy(vertex_buffer, vertex_default, VERTEX_BUF_SIZ * sizeof(td_f32));
}

void td_render_add(const td_f32* vertices, td_i32 component)
{
    memcpy(&vertex_buffer[vertex_count * 4], vertices, (size_t)component * sizeof(td_f32));
    vertex_count++;
    if(vertex_count == 3)
    {
        // Homogeneous position conversion
        term_ivec2 p1 = ndc_to_pos(vec2_init(vertex_buffer[0] / vertex_buffer[3], vertex_buffer[1] / vertex_buffer[3]), size);
        term_ivec2 p2 = ndc_to_pos(vec2_init(vertex_buffer[4] / vertex_buffer[7], vertex_buffer[5] / vertex_buffer[7]), size);
        term_ivec2 p3 = ndc_to_pos(vec2_init(vertex_buffer[8] / vertex_buffer[11], vertex_buffer[9] / vertex_buffer[11]), size);
        ptexture_draw_triangle(display_raw, size, display_channel,
            vertex_init(p1, vertex_buffer[2],  rgba_init(255, 0,   0,   255), vec2_init(0.0f, 0.0f)),
            vertex_init(p2, vertex_buffer[6],  rgba_init(0,   255, 0,   255), vec2_init(0.0f, 0.0f)),
            vertex_init(p3, vertex_buffer[10], rgba_init(0,   0,   255, 255), vec2_init(0.0f, 0.0f)),
            depth_buffer, 0);
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

TD_INLINE td_u8 rgb_to_216(const td_u8 *c) {
    return (td_u8)(16 + (c[0] / 51 * 36) + (c[1] / 51 * 6) + (c[2] / 51));
}

TD_INLINE void display_cell(td_u8 *c)
{
    if (display_channel == 1) {
        if (numeric_options[td_opt_display_type] ==
            td_display_grayscale_256)
            printf("\x1b[48;2;%d;%d;%dm", c[0], c[0], c[0]);
        else if (numeric_options[td_opt_display_type] ==
                 td_display_grayscale_24)
            printf("\x1b[48;5;%dm", 232 + ((c[0] * 24) >> 8));
    } else if (display_channel == 3) {
        if (numeric_options[td_opt_display_type] == td_display_truecolor)
            printf("\x1b[48;2;%d;%d;%dm", c[0], c[1], c[2]);
        if (numeric_options[td_opt_display_type] ==
            td_display_truecolor_216)
            printf("\x1b[48;5;%dm", rgb_to_216(c));
    }
}

// ANSI escape sequence https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
term_bool td_show()
{
    if (internal_failure)
        return 1;
    
    printf("\x1b[H");
    td_u8 *ptr = display_raw;
    static td_u8 prev[3] = { 0 };
    for (td_i32 row = display_prop.y_start; row != display_prop.y_end;
         row += display_prop.y_inc) {
        for (td_u8 i = 0; i < numeric_options[td_opt_pixel_height]; i++) {
            td_u8 *ref = &ptr[(row * size.x + display_prop.x_start)* display_channel];
            for (td_u16 col = 0; col < size.x; col++, ref += display_prop.x_inc * display_channel) {
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
    if(!td_initialized) return;
    // Show the cursor again
    // Reset color / graphics mode
    fflush(stdout);             // Flush remaining data
    _pwrite(STDOUT_FILENO, "\x1b[?25h\x1b[0m\x1b[?1049l", 19);
    restore_env();
    tdt_free(display);
    td_initialized = term_false;
}
