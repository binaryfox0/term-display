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
typedef unsigned long long u64;
typedef float f32;

#include "term_math.h"

typedef struct { u8 r, g, b; } term_rgb;
typedef struct { u8 r, g, b, a; } term_rgba;

/* Structure initializer begin */
static inline term_rgb rgb_init(const u8 r, const u8 g, const u8 b)
{
 return (term_rgb) { .r = r, .g = g, .b = b };
}

static inline term_rgba to_rgba(const term_rgb c)
{
 return (term_rgba) { .r = c.r, .g = c.g, .b = c.b, .a = 255 };
}

static inline term_rgba rgba_init(const u8 r, const u8 g, const u8 b, const u8 a)
{
 return (term_rgba) { .r = r, .g = g, .b = b , .a = a };
}

static inline term_rgb to_rgb(const term_rgba c)
{
 return (term_rgb) { .r = c.r, .g = c.g, .b = c.b };
}

/* Structure initializer end */

#endif

