#ifndef TERMINAL_MATH_H
#define TERMINAL_MATH_H

#include "term_def.h"

typedef struct {
    int x, y;
} term_ivec2;
typedef union {
    struct {
        float x, y;
    };
    float raw[2];
} term_vec2;
typedef union {
    struct {
        float x, y, z;
    };
    float raw[3];
} term_vec3;
typedef struct {
    term_u32 x, y, z;
} term_ivec3;

static inline term_ivec2 ivec2_init(int x, int y)
{
    return (term_ivec2) {
    .x = x,.y = y};
}

static inline term_vec2 vec2_init(float x, float y)
{
    return (term_vec2) {
    .x = x,.y = y};
}

static inline term_u8 vec2_equal(term_ivec2 a, term_ivec2 b)
{
    return a.x == b.x && a.y == b.y;
}

static inline term_ivec2 ndc_to_pos(term_vec2 pos, term_ivec2 size)
{
    return ivec2_init((term_i32) ((pos.x + 1.0) * 0.5f * size.x),
                      (term_i32) ((1.0 - pos.y) * 0.5f * size.y)
        );
}

#endif
