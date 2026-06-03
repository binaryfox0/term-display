/*
MIT License

Copyright (c) 2025 binaryfox0 (Duy Pham Duc)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef TD_RENDERER_H
#define TD_RENDERER_H

#include <td_def.h>
#include <td_error.h>

typedef struct td_texture td_texture_t;
typedef struct td_window td_window_t;
typedef struct td_renderer td_renderer_t;

typedef enum td_vtx_attr 
{
    TDVA_POSITION_4D,
    TDVA_POSITION_3D,
    TDVA_POSITION_2D,
    TDVA_COLOR_RGBA,
    TDVA_COLOR_RGB,
    TDVA_UV_COORDS
} td_vtx_attr_t;

td_renderer_t *td_renderer_create(
        td_window_t *window);

td_error_t td_renderer_get_size(
        td_renderer_t *renderer,
        td_i32 *w, td_i32 *h);

td_error_t td_renderer_set_draw_color(
        td_renderer_t *renderer,
        const td_u8 r,
        const td_u8 g,
        const td_u8 b,
        const td_u8 a);

/**
 * @brief Clear the framebuffer with color set by td_set_clear color and clear the depth buffer if it was enabled.
 */
td_error_t td_renderer_clear(td_renderer_t *renderer);


td_error_t td_renderer_draw_point(
        td_renderer_t *renderer,
        const td_i32 x, const td_i32 y);


td_error_t td_renderer_draw_line(
    td_renderer_t *renderer,
    const td_i32 x1, const td_i32 y1,
    const td_i32 x2, const td_i32 y2);


/**
 * @brief Draw a rectangle into framebuffer with given size
 */
td_error_t td_draw_rect(
        const td_irect rect,
        const td_rgba color);

/**
 * @brief Copy the given texture directly into the framebuffer with given placement position
 *
 * @param tex The texture to be copied into framebuffer
 * @param placement_pos The position of texture inside framebuffer 
 */

td_error_t td_renderer_copy_raw(
        const td_texture_t* texture, 
        const td_irect src_rect,
        const td_irect dst_rect);
/**
 */
td_error_t td_renderer_bind_texture(const td_texture_t* tex);
td_error_t td_add_vertex(
        const td_f32 *vertex, 
        const td_vtx_attr_t* vertex_attribs, 
        const int attribs_count, 
        const td_bool finalize);

void td_renderer_destroy(td_renderer_t *renderer);

#endif
