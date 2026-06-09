#ifndef TD_RENDERER_PRIV_H
#define TD_RENDERER_PRIV_H

#include <td_error.h>
#include <td_def.h>

#include <td_renderer.h>
#include "td_rasterizer.h"

//typedef struct td_renderer td_renderer_t;
#define TDP_VTX_BUF_SIZE 12
typedef struct td_renderer
{
    td_ivec2 size;
    td_texture_t *fb;
    td_f32 *depth_buf;
    td_rgba term_col;

    td_rgba color;

    td_renderer_mode_t current_mode;
    const td_texture_t *bound_tex;
    tdp_vertex_t vtx_buf[TDP_VTX_BUF_SIZE];
    td_i32 vtx_idx;

    td_bool depth_enable;
    td_bool wireframe_enable;
} td_renderer_t;
td_error_t tdp_renderer_resize(
        td_renderer_t *renderer,
        const td_ivec2 logical_size);

#endif

