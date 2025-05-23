#ifndef TD_TEXTURE_PRIVATE_H
#define TD_TEXTURE_PRIVATE_H

#include "td_def.h"

typedef struct
{
    td_ivec2 pos;
    td_f32 depth;
    td_rgba color;
    td_vec2 uv;
} vertex;

__td_priv_create_constructor(vertex_init, vertex, pos, depth, color, uv)

td_u8* ptexture_resize(const td_u8* old,
                     const td_u8 channel,
                     const td_ivec2 old_size,
                     const td_ivec2 new_size);

void ptexture_draw_line(td_u8 * texture,
                        const td_ivec2 size,
                        const td_u8 channel,
                        const td_ivec2 p1,
                        const td_ivec2 p2,
                        const td_vec2 depth,
                        const td_rgba color, td_f32 * depth_buffer);

void ptexture_draw_triangle(td_u8 * output,
                            const td_ivec2 size,
                            const td_u8 channel,
                            const vertex v1,
                            const vertex v2,
                            const vertex v3,
                            td_f32 * depth_buffer,
                            const td_u8* texture_data,
                            const td_ivec2 texture_size
                        );

#endif
