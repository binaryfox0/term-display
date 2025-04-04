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

typedef char term_i8;
typedef unsigned char term_u8;
typedef unsigned short term_u16;
typedef int term_i32;
typedef unsigned int term_u32;
typedef unsigned long long term_u64;
typedef float term_f32;
typedef enum { term_false = 0, term_true } term_bool;

#include "term_math.h"

typedef struct {
    term_u8 r, g, b;
} term_rgb;
typedef struct {
    term_u8 r, g, b, a;
} term_rgba;

/* Structure initializer begin */
static inline term_rgb rgb_init(const term_u8 r, const term_u8 g, const term_u8 b)
{
    return (term_rgb) {
    .r = r,.g = g,.b = b};
}

static inline term_rgba to_rgba(const term_rgb c)
{
    return (term_rgba) {
    .r = c.r,.g = c.g,.b = c.b,.a = 255};
}

static inline term_rgba rgba_init(const term_u8 r, const term_u8 g, const term_u8 b,
                                  const term_u8 a)
{
    return (term_rgba) {
    .r = r,.g = g,.b = b,.a = a};
}

static inline term_rgb to_rgb(const term_rgba c)
{
    return (term_rgb) {
    .r = c.r,.g = c.g,.b = c.b};
}

/* Structure initializer end */

#endif
