#ifndef TD_TEXTURE_PRIVATE_H
#define TD_TEXTURE_PRIVATE_H

#include "td_def.h"

struct td_texture_s;
typedef struct td_texture_s td_texture;

typedef struct
{
    td_ivec2 pos;
    td_f32 depth;
    td_rgba color;
    td_vec2 uv;
} tdr_vertex;

__td_priv_create_constructor(vertex_init, tdr_vertex, pos, depth, color, uv)

td_u8* ptexture_resize(const td_u8* old,
                     const td_u8 channel,
                     const td_ivec2 old_size,
                     const td_ivec2 new_size);

void tdr_draw_line(const td_texture* fb,
                        td_f32* depth_buf,
                        const td_ivec2 p1,
                        const td_ivec2 p2,
                        const td_rgba color);

void td_rasterize_triangle( const td_texture* fb,
                            td_f32* depth_buf,
                            const tdr_vertex v1,
                            const tdr_vertex v2,
                            const tdr_vertex v3,
                            const td_texture* tex
                        );

#endif
