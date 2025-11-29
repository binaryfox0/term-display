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
} tdp_vertex;

void tdp_draw_line(const td_texture* fb,
                        td_f32* depth_buf,
                        const td_ivec2 p1,
                        const td_ivec2 p2,
                        const td_rgba color);

void tdp_rasterize_triangle( const td_texture* fb,
                            td_f32* depth_buf,
                            const tdp_vertex v1,
                            const tdp_vertex v2,
                            const tdp_vertex v3,
                            const td_texture* tex
                        );

#endif
