#ifndef TD_PLATFORM_PRIVATE_H
#define TD_PLATFORM_PRIVATE_H

#include "td_def.h"
#include "td_main.h"

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
#endif

#ifdef TD_PLATFORM_UNIX
#include <unistd.h>

#include "td_rasterizer.h"

#define _pread read
#define _pwrite write
#define _pisatty isatty
#endif

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))

struct td_texture_s {
    td_u8 *data;
    td_u8 channel;
    td_ivec2 size;
    td_u8 freeable;
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
TD_INLINE td_u64 calculate_pos(int x, int y, td_i32 width, td_u8 ch){
    return (td_u64)((y * width + x) * ch);
}

TD_INLINE td_u64 calculate_size(int x, int y, td_u8 ch){
    return calculate_pos(0, x, y, ch);
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
void tdp_clear_screen(void);
void tdp_resize_depth_buffer(void);

#ifdef TD_PLATFORM_UNIX
td_bool setup_env(void (*handler)(int));
#else
td_bool setup_env(BOOL (*handler)(DWORD));
#endif

extern volatile td_bool td_initialized; // td_main.c
extern td_bool tdp_shift_translate;

td_ivec2 query_terminal_size(void);
td_bool restore_env(void);
void kbpoll_events(key_callback_func func);
td_bool timeout(int ms);
int available(void);

td_bool disable_stop_sig(void);
td_bool enable_stop_sig(void);

#define IS_TRANSPARENT(channel) ((channel) == 2 || (channel) == 4)
#define IS_GRAYSCALE(channel) ((channel) == 1 || (channel) == 2)
#define IS_TRUECOLOR(channel) ((channel) == 3 || (channel) == 4)


void convert(td_u8 * b_out, const td_u8 * b_in, td_u8 ch_a, td_u8 ch_b, td_u8 *out_b);
void alpha_blend(td_u8 * a, const td_u8 * b, td_u8 ch_a, td_u8 ch_b);

void fill_buffer(void* dest, const void* src, size_t destsz, size_t srcsz);

#endif
