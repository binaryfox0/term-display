#ifndef TERMINAL_TEXTURE_PRIVATE_H
#define TERMINAL_TEXTURE_PRIVATE_H

#include "term_def.h"

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
                            const term_ivec2 p1,
                            const term_ivec2 p2,
                            const term_ivec2 p3,
                            const term_rgba color, 
                            const term_vec3 depth, f32 * depth_buffer);

#endif
