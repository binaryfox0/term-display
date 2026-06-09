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
#include "td_window_priv.h"
#include "td_rasterizer.h"


TD_INLINE td_rgba tdp_query_background(void)
{
    char r[5] = {0}, b[5] = {0}, g[5] = {0};
    char buffer[32] = {0};
    td_rgba out = {0};

    tdp_tty_write("\x1b]11;?\x1b\\");
    if (!tdp_term_stdin_ready(32))
        return out;
    if (tdp_tty_read(buffer, sizeof(buffer) - 1) == -1)
        return out;
    if (sscanf(buffer, "\x1B]11;rgb:%4[^/]/%4[^/]/%4[^;]", r, g, b) != 3)
        return out;

    out.r = (td_u8)(strtol(r, 0, 16) / 257);
    out.g = (td_u8)(strtol(g, 0, 16) / 257);
    out.b = (td_u8)(strtol(b, 0, 16) / 257);
    out.a = 255;
    return out;
}
td_renderer_t *td_renderer_create(td_window_t *window)
{
    td_renderer_t *renderer = 0;
    if(!window)
        return 0;

    renderer = calloc(1, sizeof(*renderer));
    if(!renderer)
        return 0;

    renderer->fb = td_texture_create(0, TD_TEXTURE_RGB, 
            (td_ivec2){.x=window->rect.w, .y=window->rect.h}, 
            TD_FALSE, TD_FALSE);
    if(!renderer->fb)
    {
        free(renderer);
        return 0;
    }
    renderer->term_col = tdp_query_background();

    tdp_window_update_bound(window);

    window->renderer = renderer;
    return renderer;
}

/*
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
*/

td_error_t tdp_renderer_resize(
        td_renderer_t *renderer,
        const td_ivec2 logical_size)
{
    td_error_t err = TD_ERR_OK;
    if(!renderer)
        return TD_ERR_OK;

    err = td_texture_resize_buffer(renderer->fb, logical_size);
    if(err != TD_ERR_OK)
        return err; 
//    err = tdp_resize_depth_buffer(logical_size);
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
/*
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
*/
td_error_t td_renderer_get_size(
        td_renderer_t *renderer,
        td_i32 *w, td_i32 *h)
{
    if(!renderer || !w || !h)
        return TD_ERR_INVALID_ARG;
    *w = renderer->fb->size.x;
    *h = renderer->fb->size.y;
    return TD_ERR_OK;
}

td_error_t td_renderer_set_draw_color(
        td_renderer_t *renderer,
        const td_u8 r,
        const td_u8 g,
        const td_u8 b,
        const td_u8 a)
{
    td_rgba color = {0};
    if(!renderer)
        return TD_ERR_INVALID_ARG;

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    renderer->color = 
        td_blend_color(renderer->term_col, color);
    return TD_ERR_OK;
}

td_error_t td_renderer_clear(td_renderer_t *renderer)
{
    if(!renderer)
        return TD_ERR_INVALID_ARG;
        
    td_texture_fill(renderer->fb, renderer->color);
    if(renderer->depth_enable)
    {
        tdp_fill_buffer(renderer->depth_buf, &(td_f32){ FLT_MAX },
                tdp_calculate_size(renderer->size, sizeof(td_f32)),
                sizeof(td_f32));
    }
    return TD_ERR_OK;
}

td_error_t td_renderer_draw_point(
        td_renderer_t *renderer,
        const td_i32 x, const td_i32 y)
{
    td_i32 w = 0, h = 0;
    td_u8 *pixel_ptr = 0;
    if(!renderer)
        return TD_ERR_INVALID_ARG;

    w = renderer->fb->size.x;
    h = renderer->fb->size.y;
    if(x < 0 || y < 0 || x >= w || y >= h)
        return TD_ERR_OK;

    pixel_ptr = tdp_get_pixel(renderer->fb, (td_ivec2){.x=x, .y=y});
    tdp_blend(pixel_ptr, renderer->color.raw, 
            (td_i32)renderer->fb->type, 4, pixel_ptr);
    return TD_ERR_OK;
}

// Liang-Barsky line clipping algorithm
static td_error_t tdp_clip_line(
        td_ivec2 *p1, td_ivec2 *p2,
        const td_ivec2 size)
{
    td_f32 dx = (td_f32)(p2->x - p1->x);
    td_f32 dy = (td_f32)(p2->y - p1->y);

    td_f32 t0 = 0.0f;
    td_f32 t1 = 1.0f;

    td_f32 p[4], q[4];

    p[0] = -dx; q[0] = (td_f32)p1->x;             // left
    p[1] =  dx; q[1] = (td_f32)(size.x - 1 - p1->x);   // right
    p[2] = -dy; q[2] = (td_f32)p1->y;             // top
    p[3] =  dy; q[3] = (td_f32)(size.y - 1 - p1->y);   // bottom

    for (int i = 0; i < 4; i++)
    {
        if (p[i] == 0)
        {
            if (q[i] < 0)
                return TD_ERR_OK; // parallel outside
        }
        else
        {
            float r = q[i] / p[i];
            if (p[i] < 0)
            {
                if (r > t1) return TD_ERR_OK;
                if (r > t0) t0 = r;
            }
            else
            {
                if (r < t0) return TD_ERR_OK;
                if (r < t1) t1 = r;
            }
        }
    }

    p1->x = (td_i32)((td_f32)p1->x + t0 * dx);
    p1->y = (td_i32)((td_f32)p1->y + t0 * dy);
    p2->x = (td_i32)((td_f32)p2->x + t1 * dx);
    p2->y = (td_i32)((td_f32)p2->y + t1 * dy);

    return TD_ERR_OK;
}

td_error_t td_renderer_draw_line(
    td_renderer_t *renderer,
    const td_i32 x1, const td_i32 y1,
    const td_i32 x2, const td_i32 y2)
{
    tdp_vertex_t p1 = {0};
    tdp_vertex_t p2 = {0};
    if (!renderer) 
        return TD_ERR_INVALID_ARG;

    // 1. Temporary screen vectors for the 2D clipper
    td_ivec2 screen_p1 = { .x = x1, .y = y1 };
    td_ivec2 screen_p2 = { .x = x2, .y = y2 };

    // 2. Clip them directly in 2D space so the loop doesn't explode
    // (Assuming tdp_clip_line returns a boolean or handles visibility checks)
    tdp_clip_line(&screen_p1, &screen_p2, renderer->fb->size);

    p1.color = renderer->color;
    p2.color = renderer->color;

    td_f32 w = (td_f32)renderer->fb->size.x;
    td_f32 h = (td_f32)renderer->fb->size.y;

    p1.pos.x = (2.0f * (td_f32)screen_p1.x / w) - 1.0f;
    p1.pos.y = (2.0f * (td_f32)screen_p1.y / h) - 1.0f;
    p1.pos.z = 0.0f;
    p1.pos.w = 1.0f; 

    p2.pos.x = (2.0f * (td_f32)screen_p2.x / w) - 1.0f;
    p2.pos.y = (2.0f * (td_f32)screen_p2.y / h) - 1.0f;
    p2.pos.z = 0.0f;
    p2.pos.w = 1.0f;

    tdp_rasterize_line(renderer->fb, 0, p1, p2);

    return TD_ERR_OK;
}
/*
td_error_t td_renderer_draw_line(
    td_renderer_t *renderer,
    const td_i32 x1, const td_i32 y1,
    const td_i32 x2, const td_i32 y2)
{
    td_i32 w = 0, h = 0;
    tdp_vertex_t p1 = {0}, p2 = {0};
    if (!renderer)
        return TD_ERR_INVALID_ARG;

    p1.pos.x = x1;
    p1.pos.y = y1;
    p1.color = renderer->color;

    p2.pos.x = x2;
    p2.pos.y = y2;
    p2.color = renderer->color;
    
    tdp_clip_line(
            &p1.pos, &p2.pos,
            renderer->fb->size);

    tdp_rasterize_line(renderer->fb, 0, p1, p2);
    return TD_ERR_OK;
}*/

/*
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
*/
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


#define TDP_GET_VTX(renderer) ((renderer)->vtx_buf[(renderer)->vtx_idx])
td_error_t td_renderer_tex_coord_2f(
        td_renderer_t *renderer,
        const td_f32 s,
        const td_f32 t)
{
    if(!renderer)
        return TD_ERR_INVALID_ARG;

    TDP_GET_VTX(renderer).uv.x = s;
    TDP_GET_VTX(renderer).uv.y = t;
    return TD_ERR_OK;
}

td_error_t td_renderer_vertex_2f(
        td_renderer_t *renderer,
        const td_f32 x,
        const td_f32 y)
{
    if(!renderer)
        return TD_ERR_INVALID_ARG;

    (void)x;
    (void)y;
    
    renderer->vtx_idx++;
    return TD_ERR_OK;
}

/*
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
*/

void td_renderer_destroy(td_renderer_t *renderer)
{
    if(!renderer)
        return;
    td_texture_destroy(renderer->fb);
    free(renderer);
}
