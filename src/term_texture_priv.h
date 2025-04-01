#ifndef TERMINAL_TEXTURE_PRIVATE_H
#define TERMINAL_TEXTURE_PRIVATE_H

#include "term_def.h"

typedef struct
{
    term_ivec2 pos;
    f32 depth;
    term_rgba color;
} vertex;

void ptexture_draw_line(u8 * texture,
                        const term_ivec2 size,
                        const u8 channel,
                        const term_ivec2 p1,
                        const term_ivec2 p2,
                        const term_vec2 depth,
                        const term_rgba color, f32 * depth_buffer);

void ptexture_draw_triangle(u8 * texture,
                            const term_ivec2 size,
                            const u8 channel,
                            const vertex v1,
                            const vertex v2,
                            const vertex v3,
                            f32 * depth_buffer);

#endif
