#ifndef TD_CONTEXT_H
#define TD_CONTEXT_H

#include <td_def.h>
#include <td_error.h>
#include <td_option.h>

#include <td_window.h>
#include <td_renderer.h>
#include "td_rasterizer.h"

#define TDP_VTX_BUF_SIZE 12

typedef struct td_texture td_texture_t;
typedef struct tdp_renderer_context
{
    td_ivec2 size;
    td_texture_t *fb;
    td_f32 *depth_buf;

    const td_texture_t *bound_tex;
    td_rgba bg_col;
    td_rgba clear_col;
    tdp_vertex_t vtx_buf[TDP_VTX_BUF_SIZE];
    td_i32 vtx_idx;

    td_bool depth_enable;
    td_bool wireframe_enable;
} tdp_renderer_context_t;

typedef struct tdp_window_context
{
    int fb_xend;
    int fb_yend;
    
    td_ivec2 prev_size;
    
    // settings
    td_bool auto_resize;
    td_i32 rotation;
    td_ivec2 pos;
    td_ivec2 size;
    int pix_width;
    int pix_height;
    td_color_mode_t color_mode;
} tdp_window_context_t;

typedef struct tdp_context
{
    tdp_window_context_t window;
    tdp_renderer_context_t renderer;
} tdp_context_t;

extern tdp_context_t *tdp_ctx;
td_error_t tdp_context_init(void);
void tdp_context_exit(void);


#endif
