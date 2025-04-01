#ifndef TERMINAL_PRIVATE_H
#define TERMINAL_PRIVATE_H

#include "term_def.h"
#include "term_display.h"

#ifdef TERMINAL_WINDOWS
#include <io.h>
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

#ifdef TERMINAL_UNIX
#include <unistd.h>
#define _pread read
#define _pwrite write
#define _pisatty isatty
#endif

// Inline stuff
static inline u64 calculate_pos(u32 x, u32 y, u32 width, u8 ch)
{
    return (y * width + x) * ch;
}

static inline float lerp(u8 c0, u8 c1, float t)
{
    return c0 + t * (c1 - c0);
}

static inline u8 to_grayscale(const u8 *c)
{
    return (77 * c[0] + 150 * c[1] + 29 * c[2]) >> 8;
}


term_ivec2 query_terminal_size();
u8 setup_env(void *stop_handler);
u8 restore_env();
void kbpoll_events(key_callback_func func);
u8 timeout(int ms);


#define IS_TRANSPARENT(channel) ((channel) == 2 || (channel) == 4)
#define IS_GRAYSCALE(channel) ((channel) == 1 || (channel) == 2)
#define IS_TRUECOLOR(channel) ((channel) == 3 || (channel) == 4)


void convert(u8 * b_out, const u8 * b_in, u8 ch_a, u8 ch_b, u8 *out_b);
void alpha_blend(u8 * a, const u8 * b, u8 ch_a, u8 ch_b);

#endif
