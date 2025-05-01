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

#ifndef TD_DEFINITION_H
#define TD_DEFINITION_H

#include "td_black_magic.h"

#define EXPAND_RGBA(c) { (c).r, (c).g, (c).b, (c).a }
#define IN_RANGE(value, first, last) ((first) <= (value) && (value) <= (last))
#define OUT_RANGE(value, first, last) ((value) < (first) || (value) > (last))

#if defined(_WIN64) || defined(_WIN32)
#   define TD_PLATFORM_WINDOWS
#endif

//https://stackoverflow.com/a/7063372/19703526
#if defined(unix) || defined(__unix__) || defined(__unix)
#   define TD_PLATFORM_UNIX
#endif

#if defined(_MSC_VER)
#   define TD_INLINE __forceinline
#else
#   define TD_INLINE static inline __attribute((always_inline))
#endif

typedef char td_i8;
typedef unsigned char td_u8;
typedef unsigned short td_u16;
typedef int td_i32;
typedef unsigned int td_u32;
typedef unsigned long long td_u64;
typedef float td_f32;
typedef enum { term_false = 0, term_true } term_bool;

#include "td_math.h"

typedef struct {
    struct {
        td_u8 r, g, b;
    };
    td_u8 raw[3];
} term_rgb;
typedef union {
    struct {
        td_u8 r, g, b, a;
    };
    td_u8 raw[4];
} term_rgba;

/* Structure initializer begin */
__td_priv_create_constructor(rgba_init, term_rgba, r, g, b, a)
__td_priv_create_constructor(rgb_init, term_rgb, r, g, b)

TD_INLINE term_rgba to_rgba(const term_rgb c)
{
    return (term_rgba) {
    .r = c.r,.g = c.g,.b = c.b,.a = 255};
}

TD_INLINE term_rgb to_rgb(const term_rgba c)
{
    return (term_rgb) {
    .r = c.r,.g = c.g,.b = c.b};
}

/* Structure initializer end */

#endif
