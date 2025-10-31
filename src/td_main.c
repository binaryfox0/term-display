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

#include "td_black_magic.h"

/* Global variable begin */
static td_ivec2 tdp_term_size = { 0, 0 }, tdp_prev_size = { 0, 0 };
// Display clear color
td_rgba default_background = { .r = 0, .g = 0, .b = 0, .a = 255 }, 
          clear_color = { .r = 0, .g = 0, .b = 0, .a = 255};

volatile td_bool internal_failure = 0;
volatile td_bool __display_is_running = td_false,
    td_initialized = td_false;

td_i32 tdp_options[__td_opt_numeric_end__] = {
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

extern td_display tdp_display;

/* Utils function end */

const char *td_copyright_notice(void)
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

td_bool td_init(void)
{
    if(td_initialized) return td_true;
    if (!_pisatty(STDOUT_FILENO)) // Need stdout
        return 1;
    if (setup_env(stop_display))
        return 1;
    
    tdp_term_size = tdp_prev_size = query_terminal_size();      // Disable calling callback on the first time
    if(tdp_renderer_init(tdp_term_size))
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

#define td_opt_set(index, type) tdp_options[(index)] = (td_i32)(*(type*)option)
#define tdp_ensure_initialized() if(!td_initialized) return td_true

td_bool td_option(td_settings_t type, td_bool get, void *option) {
    switch (type) {
    case td_opt_auto_resize: {
        OPT_GET(td_u8, tdp_options[type]);
        OPT_SET(td_u8, tdp_options[type]);
        break;
    }

    case td_opt_pixel_width:
    case td_opt_pixel_height: {
        OPT_GET(td_u8, tdp_options[type]);
        td_u8 tmp = *(td_u8*)option;
        if(!tmp) return td_true;
        tdp_options[type] = tmp;
        tdp_resize_handle(tdp_term_size);
        break;
    }

    case td_opt_display_size: {
        OPT_GET(td_ivec2, tdp_display.fb->size);
        td_ivec2 tmp = *(td_ivec2*)option;
        if(td_ivec2_is_zero(tmp)) return td_true;
        if(!td_initialized) return td_false;
        if ((internal_failure = tdt_resize_internal(tdp_display.fb, tmp)))
            return 1;
        tdt_fill(tdp_display.fb, clear_color);
        tdp_clear_screen();
        break;
    }

    case td_opt_display_pos: {
        OPT_GET(td_ivec2, tdp_display.pos);
        OPT_SET(td_ivec2, tdp_display.pos);
        tdp_ensure_initialized();
        tdp_resize_handle(tdp_term_size);
        break;
    }

    case td_opt_display_type: {
        OPT_GET(td_display_types, tdp_options[type]);
        td_display_types tmp = *(td_display_types*)option;
        td_u8 new_channel = 3;
        switch(tmp) {
        case td_display_grayscale_24:
        case td_display_grayscale_256: new_channel = 1; break;
        case td_display_truecolor_216:
        case td_display_truecolor: new_channel = 3; break;
        default:
            return 1;
        }
        tdp_options[type] = (td_i32)tmp;
        tdp_ensure_initialized();
        tdt_set_channel(tdp_display.fb, new_channel);
        break;
    }

    case td_opt_display_rotate: {
        OPT_GET(td_u8, tdp_options[type]);
        OPT_SET(td_u8, tdp_options[type]) % 4;
        tdp_ensure_initialized();
        tdp_resize_handle(tdp_term_size);
        break;
    }

    case td_opt_depth_buffer: {
        OPT_GET(td_u8, tdp_options[type]);
        if((OPT_SET(td_u8, tdp_options[type]))){
            tdp_resize_depth_buffer();
        } else {
            if(tdp_display.depth) free(tdp_display.depth);
            tdp_display.depth = NULL;
        }
        break;
    }

    case td_opt_disable_stop_sig:
    {
        td_bool tmp = *(td_bool*)option;
        if((tmp ? disable_stop_sig() : enable_stop_sig()))
            return 1;
        tdp_options[type] = (td_i32)tmp;
        break;
    }

    case td_opt_shift_translate: {
        OPT_GET(td_bool, tdp_shift_translate);
        OPT_SET(td_bool, tdp_shift_translate);
        break;
    }

    case td_opt_enable_supersampling: {
        OPT_GET(td_bool, tdp_options[type]);
        td_opt_set(type, td_bool);
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
void td_poll_events(void)
{
    kbpoll_events(private_key_callback);
    if (!ivec2_equal((tdp_term_size = query_terminal_size()), tdp_prev_size)) {
        if (tdp_options[td_opt_auto_resize]) {
            tdp_resize_handle(tdp_term_size);
            if(private_resize_callback) private_resize_callback(tdp_display.fb->size);
        } else
            tdp_clear_screen();
        tdp_prev_size = tdp_term_size;
    }
}

void td_free(void)
{
    if(!td_initialized) return;
    tdp_renderer_exit();
    restore_env();
    td_initialized = td_false;
}
