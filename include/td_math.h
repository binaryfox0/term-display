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

#ifndef TD_MATH_H
#define TD_MATH_H

#include "td_def.h"

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
    td_u32 x, y, z;
} term_ivec3;

__td_priv_create_constructor(ivec2_init, term_ivec2, x, y)
__td_priv_create_constructor(vec2_init, term_vec2, x, y)

TD_INLINE td_u8 ivec2_equal(const term_ivec2 a, const term_ivec2 b)
{
    return a.x == b.x && a.y == b.y;
}

TD_INLINE term_ivec2 ndc_to_pos(term_vec2 pos, term_ivec2 size)
{
    return ivec2_init((td_i32) ((pos.x + 1.0) * 0.5f * size.x),
                      (td_i32) ((1.0 - pos.y) * 0.5f * size.y)
        );
}

#endif
