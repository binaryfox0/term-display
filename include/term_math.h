#ifndef TD_PLATFORM_MATH_H
#define TD_PLATFORM_MATH_H

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

__td_priv_create_constructor(ivec2_init, term_ivec2, x, y)
__td_priv_create_constructor(vec2_init, term_vec2, x, y)

TD_INLINE term_u8 ivec2_equal(const term_ivec2 a, const term_ivec2 b)
{
    return a.x == b.x && a.y == b.y;
}

TD_INLINE term_ivec2 ndc_to_pos(term_vec2 pos, term_ivec2 size)
{
    return ivec2_init((term_i32) ((pos.x + 1.0) * 0.5f * size.x),
                      (term_i32) ((1.0 - pos.y) * 0.5f * size.y)
        );
}

#endif
