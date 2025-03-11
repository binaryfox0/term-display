#ifndef TERMINAL_DEFINITION_H
#define TERMINAL_DEFINITION_H

#define EXPAND_RGBA(c) { (c).r, (c).g, (c).b, (c).a }
#define IN_RANGE(value, first, last) ((first) <= (value) && (value) <= (last))
#define OUT_RANGE(value, first, last) ((value) < (first) || (value) > (last))

#if defined(_WIN64) || defined(_WIN32)
 #define TERMINAL_WINDOWS
#endif

//https://stackoverflow.com/a/7063372/19703526
#if defined(unix) || defined(__unix__) || defined(__unix)
 #define TERMINAL_UNIX
#endif

typedef char i8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
#if defined(_WIN64) || \
    defined(__x86_64__) || \
    defined(__aarch64__)
 typedef unsigned long long u64;
#else
 typedef unsigned int u64;
#endif

#include "term_math.h"

typedef struct { u8 r, g, b; } term_rgb;
typedef struct { u8 r, g, b, a; } term_rgba;

/* Structure initializer begin */
static inline term_rgb rgb_init(u8 r, u8 g, u8 b)
{
 return (term_rgb) { .r = r, .g = g, .b = b };
}

static inline term_rgba to_rgba(term_rgb c)
{
 return (term_rgba) { .r = c.r, .g = c.g, .b = c.b, .a = 255 };
}

static inline term_rgba rgba_init(u8 r, u8 g, u8 b, u8 a)
{
 return (term_rgba) { .r = r, .g = g, .b = b , .a = a };
}

static inline term_rgb to_rgb(term_rgba c)
{
 return (term_rgb) { .r = c.r, .g = c.g, .b = c.b };
}

static inline u8 compare_vec2(term_vec2 a, term_vec2 b) { return a.x == b.x && a.y == b.y; }
/* Structure initializer end */

#endif

