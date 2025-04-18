#ifndef TD_PLATFORM_PRIVATE_H
#define TD_PLATFORM_PRIVATE_H

#include "term_def.h"
#include "term_display.h"

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
TD_INLINE term_u64 calculate_pos(int x, int y, term_i32 width, term_u8 ch)
{
    return (term_u64)((y * width + x) * ch);
}

TD_INLINE term_u64 calculate_size(int x, int y, term_u8 ch)
{
    return calculate_pos(0, x, y, ch);
}

TD_INLINE float lerp(term_f32 c0, term_f32 c1, float t)
{
    return c0 + t * (c1 - c0);
}

TD_INLINE term_u8 to_grayscale(const term_u8 *c)
{
    return (term_u8)((77 * c[0] + 150 * c[1] + 29 * c[2]) >> 8);
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


void convert(term_u8 * b_out, const term_u8 * b_in, term_u8 ch_a, term_u8 ch_b, term_u8 *out_b);
void alpha_blend(term_u8 * a, const term_u8 * b, term_u8 ch_a, term_u8 ch_b);

#endif
