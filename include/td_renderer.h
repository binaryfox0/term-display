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

typedef enum td_renderer_mode
{
    TD_RENDERER_NONE,
    TD_RENDERER_LINES,
    TD_RENDERER_TRIANGLE,
    __TD_RENDERER_MAX__
} td_renderer_mode_t;

td_renderer_t *td_renderer_create(
        td_window_t *window);

td_error_t td_renderer_bind_framebuffer(
        td_renderer_t *renderer,
        td_texture_t *tex);

td_error_t td_renderer_get_size(
        td_renderer_t *renderer,
        td_i32 *w, td_i32 *h);

td_error_t td_renderer_clear_color(
        td_renderer_t *renderer,
        const td_f32 r,
        const td_f32 g,
        const td_f32 b,
        const td_f32 a);

td_error_t td_renderer_clear(
        td_renderer_t *renderer);


td_error_t td_renderer_draw_point(
        td_renderer_t *renderer,
        const td_i32 x, const td_i32 y);


td_error_t td_renderer_draw_line(
    td_renderer_t *renderer,
    const td_i32 x1, const td_i32 y1,
    const td_i32 x2, const td_i32 y2);


td_error_t td_draw_rect(
        const td_irect rect,
        const td_rgba color);

td_error_t td_renderer_blit(
        const td_texture_t* texture, 
        const td_irect src_rect,
        const td_irect dst_rect);

td_error_t td_renderer_bind_texture(
        const td_texture_t* tex);

td_error_t td_renderer_begin(
        td_renderer_t *renderer,
        const td_renderer_mode_t mode);

td_error_t td_renderer_end(
        td_renderer_t *renderer);

td_error_t td_renderer_color_3f(
        td_renderer_t *renderer,
        const td_f32 r,
        const td_f32 g,
        const td_f32 b);

td_error_t td_renderer_tex_coord_2f(
        td_renderer_t *renderer,
        const td_f32 s,
        const td_f32 t);

td_error_t td_renderer_vertex_2f(
        td_renderer_t *renderer,
        const td_f32 x,
        const td_f32 y);

td_error_t td_renderer_vertex_3f(
        td_renderer_t *renderer,
        const td_f32 x,
        const td_f32 y,
        const td_f32 z);

td_error_t td_renderer_vertex_4f(
        td_renderer_t *renderer,
        const td_f32 x,
        const td_f32 y,
        const td_f32 z,
        const td_f32 w);

void td_renderer_destroy(td_renderer_t *renderer);

#endif
