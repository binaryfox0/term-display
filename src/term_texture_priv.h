#ifndef TERMINAL_TEXTURE_PRIVATE_H
#define TERMINAL_TEXTURE_PRIVATE_H

#include "term_def.h"

typedef struct
{
    term_ivec2 pos;
    term_f32 depth;
    term_rgba color;
} vertex;

void ptexture_draw_line(term_u8 * texture,
                        const term_ivec2 size,
                        const term_u8 channel,
                        const term_ivec2 p1,
                        const term_ivec2 p2,
                        const term_vec2 depth,
                        const term_rgba color, term_f32 * depth_buffer);

void ptexture_draw_triangle(term_u8 * texture,
                            const term_ivec2 size,
                            const term_u8 channel,
                            const vertex v1,
                            const vertex v2,
                            const vertex v3,
                            term_f32 * depth_buffer);

#endif
