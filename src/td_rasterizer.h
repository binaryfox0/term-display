#ifndef TD_TEXTURE_PRIVATE_H
#define TD_TEXTURE_PRIVATE_H

#include "td_def.h"

struct td_texture_s;
typedef struct td_texture td_texture_t;

typedef struct tdp_vertex
{
    td_ivec2 pos;
    td_f32 depth;
    td_rgba color;
    td_vec2 uv;
} tdp_vertex_t;

void tdp_rasterize_line(const td_texture_t* fb,
                        td_f32* depth_buf,
                        const tdp_vertex_t p1,
                        const tdp_vertex_t p2);

void tdp_rasterize_triangle( const td_texture_t* fb,
                            td_f32* depth_buf,
                            const tdp_vertex_t v1,
                            const tdp_vertex_t v2,
                            const tdp_vertex_t v3,
                            const td_texture_t* tex);

#endif
