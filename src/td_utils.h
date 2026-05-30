#ifndef TD_PRIVATE_H
#define TD_PRIVATE_H

#include <td_def.h>

#if defined(TD_PLATFORM_WINDOWS)
#   include <io.h>
#   include <windows.h>

#   ifndef STDIN_FILENO
#       define STDIN_FILENO _fileno(stdin)
#   endif

#   ifndef STDOUT_FILENO
#       define STDOUT_FILENO _fileno(stdout)
#   endif

#   define _pread _read
#   define _pwrite _write
#   define _pisatty _isatty

typedef BOOL (*tdp_sighand)(DWORD);

#elif defined(TD_PLATFORM_UNIX)
#   include <unistd.h>

#   define _pread read
#   define _pwrite write
#   define _pisatty isatty

typedef void (*tdp_sighand_t)(int);

#else
#   error "term-display haven't added support for this platform"
#endif

#define TDP_SWAP(a, b, type) \
    do { \
        type temp = (a);          \
        (a) = (b);                \
        (b) = temp;               \
    } while (0)

TD_INLINE td_i32 tdp_min(const td_i32 x, const td_i32 y)
{
    return x < y ? x : y;
}

TD_INLINE td_i32 tdp_max(const td_i32 x, const td_i32 y)
{
    return x > y ? x : y;
}

TD_INLINE td_i32 tdp_floor(const float x) {
    return (td_i32)x;
}

TD_INLINE td_i32 tdp_ceil(const float x) {
    td_i32 i = (td_i32)x;
    return i + (x > (td_f32)i);
}

TD_INLINE td_u64 tdp_calculate_pos(
        const td_ivec2 pos, 
        const td_i32 width, 
        const td_i32 ch)
{
    return (td_u64)((pos.y * width + pos.x) * ch);
}

TD_INLINE td_u64 tdp_calculate_size(
        const td_ivec2 size,
        const td_i32 ch)
{
    return (td_u64)(size.x * size.y * ch);
}

TD_INLINE float tdp_lerp(td_f32 c0, td_f32 c1, float t){
    return c0 + t * (c1 - c0);
}

TD_INLINE td_u8 bilerp(
        const td_u8 c00, 
        const td_u8 c10, 
        const td_u8 c01, 
        const td_u8 c11, 
        const td_f32 xt, 
        const td_f32 yt)
{
    return (td_u8)tdp_lerp(
                tdp_lerp(c00, c10, xt), 
                tdp_lerp(c01, c11, xt), 
            yt);
}

TD_INLINE td_u8 to_grayscale(const td_u8 *c){
    return (td_u8)((77 * c[0] + 150 * c[1] + 29 * c[2]) >> 8);
}
void tdp_kbpoll(void);


void tdp_fill_buffer(
        void* dest, 
        const void* src, 
        const td_u64 destsz, 
        const td_u64 srcsz);

#endif
