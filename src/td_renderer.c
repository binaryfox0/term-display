#include "td_renderer.h"
#include "td_renderer_priv.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "td_utils.h"
#include "td_context.h"
#include "td_term.h"
#include "td_texture_priv.h"
#include "td_rasterizer.h"

TD_INLINE td_rgba tdp_query_background(void)
{
    static const char *request = "\x1b]11;?\x1b\\";
    td_rgba out = {0};

    _pwrite(STDOUT_FILENO, request, strlen(request));
    if (!tdp_term_stdin_ready(4))
        return out;
    char buffer[32] = { 0 };
    if (_pread(STDIN_FILENO, buffer, 32) == -1)
        return out;
    char r[5] = { 0 }, b[5] = { 0 }, g[5] = { 0 };
    if (sscanf(buffer, "\x1B]11;rgb:%4[^/]/%4[^/]/%4[^;]", r, g, b) != 3)
        return out;

    out.r = (td_u8)(strtol(r, 0, 16) / 257);
    out.g = (td_u8)(strtol(g, 0, 16) / 257);
    out.b = (td_u8)(strtol(b, 0, 16) / 257);
    out.a = 255;
    return out;
}

TD_INLINE void tdp_reset_depth_buffer(void) 
{
    tdp_renderer_context_t *renderer = &tdp_ctx->renderer;
    tdp_fill_buffer(renderer->depth_buf, &(td_f32){ FLT_MAX },
            tdp_calculate_size(renderer->size, sizeof(td_f32)),
            sizeof(td_f32));
}

td_error_t tdp_resize_depth_buffer(const td_ivec2 new_size)
{
    tdp_renderer_context_t *renderer = &tdp_ctx->renderer;
    td_u64 old_alloc_size = 0;
    td_u64 alloc_size = 0;
    td_f32 *tmp = 0;
    if(!renderer->depth_enable)
        return TD_ERR_OK;

    old_alloc_size = tdp_calculate_size(renderer->size, sizeof(td_f32));
    alloc_size = tdp_calculate_size(new_size, sizeof(td_f32));
    tmp = (td_f32 *)realloc(renderer->depth_buf, alloc_size);
    if (!tmp)
        return TD_ERR_OUT_OF_MEMORY;

    if(alloc_size > old_alloc_size)
    {
        tdp_fill_buffer(tmp + old_alloc_size, &(td_f32){FLT_MAX},
                alloc_size - old_alloc_size, sizeof(td_f32));
    }

    renderer->depth_buf = tmp;
    return TD_ERR_OK;
}

td_error_t tdp_renderer_resize(const td_ivec2 logical_size)
{
    tdp_renderer_context_t *renderer = &tdp_ctx->renderer;
    td_error_t err = TD_ERR_OK;
    err = td_texture_resize_buffer(renderer->fb, logical_size);
    if(err != TD_ERR_OK)
        return err; 
    err = tdp_resize_depth_buffer(logical_size);
    if(err != TD_ERR_OK)
    {
        // shrink in order not to nake rasterizer confuse
        renderer->size = renderer->size;
        return err;
    }
    else
        renderer->size = logical_size;
    return TD_ERR_OK;
}

td_error_t tdp_renderer_init(void)
{
    tdp_renderer_context_t *renderer = &tdp_ctx->renderer;

    renderer->clear_col = tdp_query_background();
    renderer->bg_col = renderer->clear_col; 
    renderer->fb = td_texture_create_empty(TD_TEXTURE_RGB_ALPHA);
    if(!renderer->fb)
        return TD_ERR_OUT_OF_MEMORY;
    renderer->fb->lib_owned = TD_TRUE;
    
    return TD_ERR_OK;
}

void tdp_renderer_exit(void)
{
    tdp_renderer_context_t *renderer = &tdp_ctx->renderer;
    
    renderer->fb->lib_owned = TD_FALSE;
    td_texture_destroy(renderer->fb);
    if(renderer->depth_enable)
        free(renderer->depth_buf);
}

td_error_t td_renderer_set_clear_color(const td_rgba clear_color)
{
    tdp_renderer_context_t *renderer = 0;
    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;

    renderer = &tdp_ctx->renderer;
    renderer->clear_col = 
        td_blend_pixel(renderer->bg_col, clear_color);
    return TD_ERR_OK;
}

td_error_t td_renderer_clear(void)
{
    tdp_renderer_context_t *renderer = 0;
    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;
    renderer = &tdp_ctx->renderer;
    td_texture_fill(renderer->fb, renderer->clear_col);
    tdp_reset_depth_buffer();
    return TD_ERR_OK;
}

td_texture_t *td_get_framebuffer(void) {
    if(!tdp_ctx)
        return 0;
    return tdp_ctx->renderer.fb;
}
td_error_t td_draw_rect(
        const td_irect rect,
        const td_rgba color
)
{
    tdp_renderer_context_t *renderer = 0;
    tdp_vertex_t v0 = {0}, v1 = {0}, v2 = {0}, v3 = {0};

    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;

    renderer = &tdp_ctx->renderer;

    v0.pos.x = rect.x;
    v0.pos.y = rect.y;
    v0.color = color;

    v1.pos.x = rect.x + rect.w;
    v1.pos.y = rect.y;
    v1.color = color;

    v2.pos.x = rect.x;
    v2.pos.y = rect.y + rect.h;
    v2.color = color;

    v3.pos.x = rect.x + rect.w;
    v3.pos.y = rect.y + rect.h;
    v3.color = color;

    tdp_rasterize_triangle(renderer->fb, 0, v0, v1, v2, 0);
    tdp_rasterize_triangle(renderer->fb, 0, v1, v3, v2, 0);
    return TD_ERR_OK;
}

td_error_t td_renderer_copy_raw(
        const td_texture_t* texture, 
        const td_irect src_rect,
        const td_irect dst_rect)

{
    (void)texture;
    (void)src_rect;
    (void)dst_rect;
    /*
    td_texture_merge(tdp_ctx->renderer.fb, texture, 
            src_rect, dst_rect, );
    */
    return TD_ERR_OK;
}


td_error_t td_renderer_copy(
        const td_texture_t* texture, 
        const td_irect src_rect,
        const td_irect dst_rect)
{
    (void)texture;
    (void)src_rect;
    (void)dst_rect;
    /*
    td_texture_merge(tdp_ctx->renderer.fb, tex, 
            placement_pos,TD_FALSE);
    */
    return TD_ERR_OK;
}

td_error_t td_renderer_bind_texture(const td_texture_t *texture)
{
    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;
    tdp_ctx->renderer.bound_tex = texture;
    return TD_ERR_OK;
}

td_error_t td_add_vertex(
        const td_f32 *vertices, 
        const td_vtx_attr_t* vertex_attribs, 
        const int attribs_count, 
        const td_bool finalize
)
{
    tdp_renderer_context_t *renderer = 0;
    tdp_vertex_t *curr = 0;
    if(!vertices || !vertex_attribs || attribs_count <= 0)
        return TD_ERR_INVALID_ARG;
    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;

    renderer = &tdp_ctx->renderer;
    curr = &renderer->vtx_buf[renderer->vtx_idx];
    for(int i = 0; i < attribs_count; i++)
    {
        switch(vertex_attribs[i])
        {
        case TDVA_POSITION_4D:
            curr->pos = ndc_to_pos(
                (td_vec2){
                    .x = vertices[0] / vertices[3], 
                    .y = vertices[1] / vertices[3]
                },
                renderer->size
            );
            curr->depth = vertices[2];
            vertices += 4;
            break;

        case TDVA_POSITION_3D:
            curr->pos = ndc_to_pos(
                    (td_vec2){.x=vertices[0], .y=vertices[1]}, 
                    renderer->size);
            curr->depth = vertices[2];
            vertices += 3;
            break;
        
        case TDVA_POSITION_2D:
            curr->pos = ndc_to_pos((td_vec2){.x=vertices[0], .y=vertices[1]}, 
                    renderer->size);
            vertices += 2;
            break;

        case TDVA_COLOR_RGBA:
            curr->color.r = (td_u8)(vertices[0] * 255);
            curr->color.g = (td_u8)(vertices[1] * 255);
            curr->color.b = (td_u8)(vertices[2] * 255);
            curr->color.a = (td_u8)(vertices[3] * 255);
            vertices += 4;
            break;

        case TDVA_COLOR_RGB:
            curr->color.r = (td_u8)(vertices[0] * 255);
            curr->color.g = (td_u8)(vertices[1] * 255);
            curr->color.b = (td_u8)(vertices[2] * 255);
            curr->color.a = 255;
            vertices += 3;
            break;

        
        case TDVA_UV_COORDS:
            curr->uv = (td_vec2){.x=vertices[0], .y=vertices[1]};
            vertices += 2;
            break;

        default:
            return TD_ERR_INVALID_ARG;
        }
    }
    if(finalize)
        renderer->vtx_idx++;

    if(renderer->vtx_idx == 3) 
    {
        if(renderer->wireframe_enable)
        {
            tdp_rasterize_line(
                    renderer->fb, 
                    renderer->depth_buf, 
                    renderer->vtx_buf[0],
                    renderer->vtx_buf[1]
                    );
            tdp_rasterize_line(
                    renderer->fb, 
                    renderer->depth_buf, 
                    renderer->vtx_buf[1],
                    renderer->vtx_buf[2]
                    );
            tdp_rasterize_line(
                    renderer->fb, 
                    renderer->depth_buf, 
                    renderer->vtx_buf[2],
                    renderer->vtx_buf[0]
                    );
        } else {
            tdp_rasterize_triangle(
                    renderer->fb,
                    renderer->depth_buf,
                    renderer->vtx_buf[0],
                    renderer->vtx_buf[1],
                    renderer->vtx_buf[2],
                    renderer->bound_tex
            );
        }
        renderer->vtx_idx = 0;
    }
    return TD_ERR_OK;
}
