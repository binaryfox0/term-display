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

#define _pread read
#define _pwrite write
#define _pisatty isatty
#endif

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

TD_INLINE td_u8 to_grayscale(const td_u8 *c){
    return (td_u8)((77 * c[0] + 150 * c[1] + 29 * c[2]) >> 8);
}


#ifdef TD_PLATFORM_UNIX
term_bool setup_env(void (*handler)(int));
#else
term_bool setup_env(BOOL (*handler)(DWORD));
#endif

extern term_bool shift_translate;

term_ivec2 query_terminal_size();
term_bool restore_env();
void kbpoll_events(key_callback_func func);
term_bool timeout(int ms);
int available();

term_bool disable_stop_sig();
term_bool enable_stop_sig();

#define IS_TRANSPARENT(channel) ((channel) == 2 || (channel) == 4)
#define IS_GRAYSCALE(channel) ((channel) == 1 || (channel) == 2)
#define IS_TRUECOLOR(channel) ((channel) == 3 || (channel) == 4)


void convert(td_u8 * b_out, const td_u8 * b_in, td_u8 ch_a, td_u8 ch_b, td_u8 *out_b);
void alpha_blend(td_u8 * a, const td_u8 * b, td_u8 ch_a, td_u8 ch_b);

void fill_buffer(void* dest, const void* src, size_t destsz, size_t srcsz);
void reset_buffer(const void** out_buffer, const term_vec2 size, const term_vec2* out_size, const int type_size);

#endif
