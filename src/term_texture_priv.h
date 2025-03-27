#ifndef TERMINAL_TEXTURE_PRIVATE_H
#define TERMINAL_TEXTURE_PRIVATE_H

#include "term_def.h"

void ptexture_draw_line(u8 * texture,
                        const u32 width,
                        const u8 channel,
                        const term_ivec3 p1,
                        const term_ivec3 p2,
                        const term_rgba color, f32 * depth_buffer);

void ptexture_draw_triangle(u8 * texture,
                            const u32 width,
                            const u8 channel,
                            const term_ivec3 p1,
                            const term_ivec3 p2,
                            const term_ivec3 p3,
                            const term_rgba color, f32 * depth_buffer);

#endif
