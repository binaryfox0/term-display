#ifndef TD_PLATFORM_PRIVATE_H
#define TD_PLATFORM_PRIVATE_H

#include "td_def.h"
#include "td_main.h"

#include "td_rasterizer.h"
#include "td_debug.h" // IWYU pragma: export

#ifdef TD_PLATFORM_WINDOWS
#include <io.h>
#include <windows.h>

#ifndef STDIN_FILENO
#define STDIN_FILENO _fileno(stdin)
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO _fileno(stdout)
#endif

#define _pread _read
#define _pwrite _write
#define _pisatty _isatty

typedef BOOL (*tdp_sighand)(DWORD);

#endif

#ifdef TD_PLATFORM_UNIX
#include <unistd.h>

#define _pread read
#define _pwrite write
#define _pisatty isatty

typedef void (*tdp_sighand)(int);
#endif

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))

struct td_texture_s {
    td_u8 *data;
    td_i32 channel;
    td_ivec2 size;
    td_bool freeable;
};

typedef struct td_display_s {
    td_texture* fb;
    td_f32* depth;
    td_ivec2 pos;
    struct {
        int xend, yend;
    } sprop;
} td_display;

// Inline stuff
TD_INLINE td_i32 tdp_floor(const float x) {
    return (td_i32)x;
}

TD_INLINE td_i32 tdp_ceil(const float x) {
    td_i32 i = (td_i32)x;
    return i + (x > (td_f32)i);
}

TD_INLINE td_u64 calculate_pos(const td_ivec2 pos, const td_i32 width, const td_i32 ch){
    return (td_u64)((pos.y * width + pos.x) * ch);
}

TD_INLINE float lerp(td_f32 c0, td_f32 c1, float t){
    return c0 + t * (c1 - c0);
}

TD_INLINE td_u8 bilerp(td_u8 c00, td_u8 c10, td_u8 c01, td_u8 c11, float xt, float yt)
{
    return (td_u8) lerp(lerp(c00, c10, xt), lerp(c01, c11, xt), yt);
}

TD_INLINE td_u8 to_grayscale(const td_u8 *c){
    return (td_u8)((77 * c[0] + 150 * c[1] + 29 * c[2]) >> 8);
}

// td_renderer.c
int tdp_renderer_init(const td_ivec2 term_size);
void tdp_renderer_exit(void);
void tdp_resize_handle(const td_ivec2 term_size);
void tdp_resize_depth_buffer(void);

extern volatile td_bool td_initialized; // td_main.c
extern td_i32 tdp_options[__td_opt_numeric_end__]; // td_main.c
extern td_bool tdp_shift_translate;

// Platform-dependent
td_ivec2 tdp_get_termsz(void);
td_bool tdp_setup_env(const tdp_sighand handler);
void tdp_restore_env(void);
td_bool tdp_stdin_ready(const int ms);
int tdp_stdin_available(void);
td_bool tdp_enable_stop_sig(const td_bool enable);

// Platform-independent
void tdp_kbpoll(const td_key_callback keycb, const td_mouse_callback mousecb);

#define IS_TRANSPARENT(channel) ((channel) == 2 || (channel) == 4)
#define IS_GRAYSCALE(channel) ((channel) == 1 || (channel) == 2)
#define IS_TRUECOLOR(channel) ((channel) == 3 || (channel) == 4)


void tdp_convert_color(td_u8 * b_out, const td_u8 * b_in, td_i32 ch_a, td_i32 ch_b, td_i32 *out_b);
void tdp_blend(td_u8 * a, const td_u8 * b, td_i32 ch_a, td_i32 ch_b);

void tdp_fill_buffer(void* dest, const void* src, td_u64 destsz, td_u64 srcsz);

#endif
