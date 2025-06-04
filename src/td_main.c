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

#include "td_black_magic.h"

/* Global variable begin */
static struct {
    td_texture* texture;
    td_u8* raw;
    td_u8 ch;
    td_ivec2 pos;
    td_ivec2 size;
    struct {
        int xend, yend;
    } sprop;
} display = { 0, 0, 3, {0, 0}, {0, 0}, {0, 0}};
static td_ivec2 term_size = { 0, 0 }, prev_size = { 0, 0 };
// Display clear color
td_rgba default_background = { .r = 0, .g = 0, .b = 0, .a = 255 }, 
          clear_color = { .r = 0, .g = 0, .b = 0, .a = 255};

volatile td_u8 internal_failure = 0;
volatile td_bool __display_is_running = td_false,
    td_initialized = td_false;

td_f32 *depth_buffer = 0;

td_i32 numeric_options[__td_opt_numeric_end__] = {
    [td_opt_auto_resize] = 0,
    [td_opt_pixel_width] = 2,
    [td_opt_pixel_height] = 1,
    [td_opt_display_type] = td_display_truecolor,
    [td_opt_display_rotate] = 0,
    [td_opt_depth_buffer] = 0,
    [td_opt_shift_translate] = 1,
    [td_opt_enable_supersampling] = 0
};

td_f32 supersampling_buffer_ratio = 2.0f;

/* Global variable end */

TD_INLINE void reset_depth_buffer() {
    fill_buffer(depth_buffer, &(td_f32){ FLT_MAX }, calculate_size(td_ivec2_expand(display.size), sizeof(td_f32)), sizeof(td_f32));
}

TD_INLINE void clear_screen() {
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
    if(numeric_options[td_opt_display_rotate] % 2 == 0) {
        display.size =
            td_ivec2_init(term_size.x / numeric_options[td_opt_pixel_width],
                       term_size.y / numeric_options[td_opt_pixel_height]);
        display.sprop.yend = display.size.y;
        display.sprop.xend = display.size.x;
    } else {
        display.size =
            td_ivec2_init(term_size.y / numeric_options[td_opt_pixel_height],
                       term_size.x / numeric_options[td_opt_pixel_width]);
        display.sprop.yend = display.size.x;
        display.sprop.xend = display.size.y;
    }
    //display.size = td_ivec2_subtract(display.size, display.pos);
    
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

void resize_depth_buffer()
{
    if (!numeric_options[td_opt_depth_buffer])
        return;

    if (td_ivec2_is_zero(display.size)) {
        if (depth_buffer) {
            free(depth_buffer);
            depth_buffer = NULL;
        }
        return;
    }

    td_u64 buf_size = calculate_size(td_ivec2_expand(display.size), sizeof(td_f32));
    td_f32 *tmp = (td_f32 *) realloc(depth_buffer, buf_size);
    if (!tmp) {
        internal_failure = 1;
        return;
    }

    depth_buffer = tmp;
    reset_depth_buffer();
}


void resize_display()
{
    if ((internal_failure = tdt_resize_internal(display.texture, display.size)))
        return;                 // Uhhhh, how to continue processing without the display
    display.raw = tdt_get_location(td_ivec2_init(0, 0), display.texture);
    tdt_fill(display.texture, clear_color);
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
            __display_is_running = td_false;
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
    __display_is_running = td_false;
}
#endif

td_bool td_init()
{
    if(td_initialized) return td_true;
    if (!_pisatty(STDOUT_FILENO)) // Need stdout
        return 1;
    if (setup_env(stop_display))
        return 1;
    
    term_size = prev_size = query_terminal_size();      // Disable calling callback on the first time
    query_default_background();
    clear_color = default_background;   // Look like the background
    _pwrite(STDOUT_FILENO, "\x1b[?25l\x1b[?1049h", 15);     // Hide cursor and enable buffer

    if (!
        (display.texture =
        tdt_create(0, display.ch, td_ivec2_init(1, 1), 0, 0))) {
        internal_failure = 1;
        return 1;
    }
    if(td_ivec2_is_zero(display.size))
        calculate_display_size();
    resize_display();

    if (internal_failure)
        return 1;
    __display_is_running = td_true;
    td_initialized = td_true;
    return 0;
}

TD_INLINE td_bool opt_get(td_bool get, void *option, void *value, size_t s) {
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

td_bool td_option(td_settings_t type, td_bool get, void *option) {
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
        if(!tmp) return td_true;
        numeric_options[type] = tmp;
        calculate_display_size();
        resize_display();
        break;
    }

    case td_opt_display_size: {
        OPT_GET(td_ivec2, display.size);
        td_ivec2 tmp = *(td_ivec2*)option;
        if(td_ivec2_is_zero(tmp)) return td_true;
        display.size = tmp;
        if(!td_initialized) return td_false;
        if ((internal_failure = tdt_resize_internal(display.texture, display.size)))
            return 1;
        tdt_fill(display.texture, clear_color);
        clear_screen();
        break;
    }

    case td_opt_display_pos: {
        OPT_GET(td_ivec2, display.pos);
        OPT_SET(td_ivec2, display.pos);
        calculate_display_size();
        resize_display();
        break;
    }

    case td_opt_display_type: {
        OPT_GET(td_display_types, numeric_options[type]);
        td_display_types tmp = *(td_display_types*)option;
        switch(tmp) {
        case td_display_grayscale_24:
        case td_display_grayscale_256: display.ch = 1; break;
        case td_display_truecolor_216:
        case td_display_truecolor: display.ch = 3; break;
        default:
            return 1;
        }
        numeric_options[type] = tmp;
        if(td_initialized) tdt_set_channel(display.texture, display.ch);
        break;
    }

    case td_opt_display_rotate: {
        OPT_GET(td_u8, numeric_options[type]);
        OPT_SET(td_u8, numeric_options[type]) % 4;
        calculate_display_size();
        resize_display();
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
        td_bool tmp = *(td_bool*)option;
        if((tmp ? disable_stop_sig() : enable_stop_sig()))
            return 1;
        numeric_options[type] = tmp;
        break;
    }

    case td_opt_shift_translate: {
        OPT_GET(td_bool, shift_translate);
        OPT_SET(td_bool, shift_translate);
        break;
    }

    case td_opt_enable_supersampling: {
        OPT_GET(td_bool, numeric_options[type]);
        OPT_SET(td_bool, numeric_options[type]);
        break;
    }

    case td_opt_supersampling_buffer_ratio: {
        OPT_GET(td_f32, supersampling_buffer_ratio);
        OPT_SET(td_f32, supersampling_buffer_ratio);
        break;
    }

    default:
        return 1;
    }

    return 0;
}


// so much macro
#define __handler_helper(name) \
    __td_cat(name, _callback_func) __td_cat(__td_cat(private_, name), _callback); \
    void __td_cat(__td_cat(td_set_, name), _callback)(__td_cat(name, _callback_func) callback) { \
        __td_cat(__td_cat(private_, name), _callback) = callback; \
    }
__handler_helper(key)
__handler_helper(resize)
void td_poll_events()
{
    kbpoll_events(private_key_callback);
    if (!ivec2_equal((term_size = query_terminal_size()), prev_size)) {
        if (numeric_options[td_opt_auto_resize]) {
            calculate_display_size();
            resize_display();
            if(private_resize_callback) private_resize_callback(display.size);
        } else
            clear_screen();
        prev_size = term_size;
    }
}



void td_set_color(td_rgba color)
{
    clear_color = pixel_blend(default_background, color);
    tdt_fill(display.texture, clear_color);
    reset_depth_buffer();
}

void td_copy_texture(const td_texture *texture,
                          const td_vec2 pos,
                          const enum tdt_merge_mode mode)
{
    td_ivec2 display_pos = ndc_to_pos(pos, display.size);
    tdt_merge(display.texture, texture, display_pos, mode, 0);
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
        td_ivec2 p1 = ndc_to_pos(td_vec2_init(vertex_buffer[0] / vertex_buffer[3], vertex_buffer[1] / vertex_buffer[3]),   display.size);
        td_ivec2 p2 = ndc_to_pos(td_vec2_init(vertex_buffer[4] / vertex_buffer[7], vertex_buffer[5] / vertex_buffer[7]),   display.size);
        td_ivec2 p3 = ndc_to_pos(td_vec2_init(vertex_buffer[8] / vertex_buffer[11], vertex_buffer[9] / vertex_buffer[11]), display.size);
        ptexture_draw_triangle(display.raw, display.size, display.ch,
            vertex_init(p1, vertex_buffer[2],  rgba_init(255, 0,   0,   255), td_vec2_init(0.0f, 0.0f)),
            vertex_init(p2, vertex_buffer[6],  rgba_init(0,   255, 0,   255), td_vec2_init(0.0f, 0.0f)),
            vertex_init(p3, vertex_buffer[10], rgba_init(0,   0,   255, 255), td_vec2_init(0.0f, 0.0f)),
            depth_buffer, 0, td_ivec2_init(0, 0));
        vertex_count = 0;
        td_render_flush();
    }
}

void td_draw_line(const td_vec2 p1, const td_vec2 p2,
                       const td_rgba color)
{
    ptexture_draw_line(display.raw, display.size, display.ch,
                       ndc_to_pos(p1, display.size),
                       ndc_to_pos(p2, display.size), 
                       td_vec2_init(0.0f, 0.0f),color, 0);
}

TD_INLINE td_u8 rgb_to_216(const td_u8 *c) {
    return (td_u8)(16 + (c[0] / 51 * 36) + (c[1] / 51 * 6) + (c[2] / 51));
}

TD_INLINE void display_cell(td_u8 *c)
{
    const int ch = display.ch;
    const int dtype = numeric_options[td_opt_display_type];

    if (ch == 1) {
        if (dtype == td_display_grayscale_256) {
            int v = c[0];
            printf("\x1b[48;2;%d;%d;%dm", v, v, v);
        } else if (dtype == td_display_grayscale_24) {
            printf("\x1b[48;5;%dm", 232 + ((c[0] * 24) >> 8));
        }
    } else if (ch == 3) {
        if (dtype == td_display_truecolor) {
            printf("\x1b[48;2;%d;%d;%dm", c[0], c[1], c[2]);
        } else if (dtype == td_display_truecolor_216) {
            printf("\x1b[48;5;%dm", rgb_to_216(c));
        }
    }
}


// ANSI escape sequence https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
void td_show()
{
    printf("\x1b[H");
    int rot = numeric_options[td_opt_display_rotate];
    static td_u8 prev[3] = {0};

    for (int y = 0; y < display.sprop.yend; y++) {
        for (int yt = 0; yt < numeric_options[td_opt_pixel_height]; yt++) {

            for (int x = 0; x < display.sprop.xend; x++) {
                int tx = x, ty = y;

                switch (rot) {
                    case 1: /* 90° clockwise */      tx = y; ty = display.size.y - 1 - x; break;
                    case 2: /* 180° */               tx = display.size.x - 1 - x; ty = display.size.y - 1 - y; break;
                    case 3: /* 270° clockwise */     tx = display.size.x - 1 - y; ty = x; break;
                    default:                         break;
                }

                td_u8* ptr = display.raw + calculate_pos(tx, ty, display.size.x, display.ch);
                if (memcmp(prev, ptr, display.ch)) {
                    display_cell(ptr);
                    memcpy(prev, ptr, display.ch);
                }

                printf("%*s", numeric_options[td_opt_pixel_width], "");
            }

            printf("\x1b[1E");
        }
    }
}

void td_free()
{
    if(!td_initialized) return;
    // Show the cursor again
    // Reset color / graphics mode
    fflush(stdout);             // Flush remaining data
    _pwrite(STDOUT_FILENO, "\x1b[?25h\x1b[0m\x1b[?1049l", 19);
    restore_env();
    tdt_free(display.texture);
    td_initialized = td_false;
}
