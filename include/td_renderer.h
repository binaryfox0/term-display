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

#include "td_def.h"

struct td_texture_s;
typedef struct td_texture_s td_texture;

typedef enum tdr_vertex_attrib_e {
    TDRVA_POSITION_4D,
    TDRVA_POSITION_3D,
    TDRVA_POSITION_2D,
    TDRVA_COLOR_RGBA,
    TDRVA_COLOR_RGB,
    TDRVA_UV_COORDS
} tdr_vertex_attrib;

/** 
 * @brief Set clear color for tdr_clear_framebuffer
 *
 * @param clear_color The color will be used to clear framebuffer
 */
void tdr_set_clear_color(const td_rgba clear_color);

/**
 * @brief Clear the framebuffer with color set by tdr_set_clear color and clear the depth buffer if it was enabled.
 */
void tdr_clear_framebuffer(void);

/**
 * @brief Copy the given texture directly into the framebuffer with given placement position
 *
 * @param tex The texture to be copied into framebuffer
 * @param placement_pos The position of texture inside framebuffer 
 */
void tdr_copy_texture(const td_texture* tex, const td_ivec2 placement_pos);

/**
 */
void tdr_bind_texture(const td_texture* tex);
void tdr_add_vertex(const td_f32 *vertex, const tdr_vertex_attrib* vertex_attribs, const int attribs_count, const td_bool finalize);

/**
 * @brief Render built-in framebuffer
 */
void tdr_render(void);

#endif
