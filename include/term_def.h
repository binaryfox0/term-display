#ifndef TERMINAL_DEFINITION_H
#define TERMINAL_DEFINITION_H

#define EXPAND_RGBA(c) { (c).r, (c).g, (c).b, (c).a }

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

struct term_rgb { u8 r, g, b; };
struct term_rgba { u8 r, g, b, a; };
struct term_pos { float x, y; };
struct term_vec2 { u32 x, y; };

/* Structure initializer begin */
static inline struct term_rgb rgb_init(u8 r, u8 g, u8 b)
{
 return (struct term_rgb) { .r = r, .g = g, .b = b };
}

static inline struct term_rgba to_rgba(struct term_rgb c)
{
 return (struct term_rgba) { .r = c.r, .g = c.g, .b = c.b, .a = 255 };
}

static inline struct term_rgba rgba_init(u8 r, u8 g, u8 b, u8 a)
{
 return (struct term_rgba) { .r = r, .g = g, .b = b , .a = a };
}

static inline struct term_rgb to_rgb(struct term_rgba c)
{
 return (struct term_rgb) { .r = c.r, .g = c.g, .b = c.b };
}

static inline struct term_pos pos_init(float x, float y)
{
 return (struct term_pos) { .x = x, .y = y };
}

static inline struct term_vec2 vec2_init(u32 x, u32 y)
{
 return (struct term_vec2) { .x = x, .y = y };
}
/* Structure initializer end */

#endif

