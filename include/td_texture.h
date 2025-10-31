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

/**
 * @file td_texture.h
 * @brief Texture header of term-display used to control all operation related to texture.
 *
 * This file provides basic functionality for texture manipulation and graphics-related operations.
 * It includes functions for creating, resizing, cropping, merging, line drawing, and more.
 */

#ifndef TD_TEXTURE_H
#define TD_TEXTURE_H

#include "td_def.h"

struct td_texture_s;
typedef struct td_texture_s td_texture;

/**
 * @defgroup color_channel Supported Color Channels
 * @brief Supported image channel formats.
 *
 * td_texture uses the same channel layout as `stbi_load()` from stb_image.h.
 *
 * | Channels | Description               |
 * |----------|---------------------------|
 * | 1        | Grayscale                 |
 * | 2        | Grayscale with alpha      |
 * | 3        | RGB (Red-Green-Blue)      |
 * | 4        | RGBA (RGB with alpha)     |
 */

/**
 * @brief Modes for merging two textures.
 *
 * These modes control how `texture_b` is combined with `texture_a` in `tdt_merge()`.
 */
enum tdt_merge_mode {
    TDT_MERGE_CROP = 0,        ///< Crop B to fit inside A.
    TDT_MERGE_RESIZE,          ///< Resize B to match size of A.
};

/**
 * @brief Create a new td_texture.
 *
 * @param texture The raw texture data (8-bit format).
 * @param channel Number of channels (1–4). See \ref color_channel.
 * @param size Width and height of the texture in pixels.
 * @param freeable If true, `tdt_free()` will free the raw data.
 * @param copy If true, the texture data will be copied internally.
 * @return Pointer to a newly created texture object.
 *
 * @details
 * If `texture` is NULL, the function allocates memory and zero-initializes it.
 * If `copy` is true, the function copies the content of `texture` into newly
 * allocated memory. Ownership of `texture` is transferred if `freeable` is set.
 * If `size.x` or `size.y` is 0, the function returns an empty td_texture object
 * without allocating any data.
 */
td_texture *tdt_create(td_u8 * texture,
                       const td_u8 channel,
                       const td_ivec2 size,
                       const td_u8 freeable,
                       const td_u8 copy);

/**
 * @brief Create a deep copy of a texture.
 *
 * @param texture The texture to copy.
 * @return A newly allocated copy of the texture.
 */
td_texture *tdt_copy(td_texture * texture);

/**
 * @brief Get the address of a pixel at the given position.
 *
 * @param pos Pixel coordinates.
 * @param texture The texture to query.
 * @return Pointer to the pixel data (channel count depends on the texture).
 */
td_u8 *tdt_get_location(const td_ivec2 pos, const td_texture * texture);

/**
 * @brief Get the dimensions of a texture.
 *
 * @param texture The texture to query.
 * @return A vector containing the width and height.
 */
td_ivec2 tdt_get_size(const td_texture * texture);

/**
 * @brief Fill the entire texture with a single color.
 *
 * @param texture The target texture.
 * @param color The color to use. Alpha blending is supported.
 */
void tdt_fill(const td_texture * texture, const td_rgba color);

/**
 * @brief Change the texture to a new number of channels.
 *
 * @param texture The texture to convert.
 * @param channel The desired number of channels (1–4).
 */
void tdt_set_channel(td_texture * texture, td_u8 channel);

/**
 * @brief Merge two textures together.
 *
 * `texture_b` is drawn onto `texture_a` using the specified merge mode.
 *
 * @param texture_a The base texture.
 * @param texture_b The texture to draw over A.
 * @param placment_pos Position to place B on A.
 * @param mode Merge mode. See \ref tdt_merge_mode.
 * @param replace Whether to overwrite A directly (used with some modes).
 */
void tdt_merge(const td_texture * texture_a,
               const td_texture * texture_b,
               const td_ivec2 placment_pos,
               const enum tdt_merge_mode mode,
               const td_bool replace);

/**
 * @brief Resize the texture using bilinear interpolation.
 *
 * @param texture The texture to resize.
 * @param new_size The desired size.
 */
void tdt_resize(td_texture * texture, const td_ivec2 new_size);

/**
 * @brief Resize only the raw storage (no content update).
 *
 * @param texture The texture to resize.
 * @param new_size The new size.
 * @return True if successful.
 */
td_bool tdt_resize_internal(td_texture * texture, const td_ivec2 new_size);

/**
 * @brief Crop the texture to a new size.
 *
 * @param texture The texture to crop.
 * @param new_size The new size.
 */
void tdt_crop(td_texture * texture, const td_ivec2 new_size);

/**
 * @brief Draw a line onto a texture.
 *
 * @param texture The texture to draw on.
 * @param p1 Starting point.
 * @param p2 Ending point.
 * @param color Line color.
 */
void tdt_draw_line(td_texture * texture,
                   const td_ivec2 p1,
                   const td_ivec2 p2,
                   const td_rgba color);

/**
 * @brief Free the memory associated with a texture.
 *
 * @param texture The texture to free.
 */
void tdt_free(td_texture * texture);

/**
 * @brief Blend two pixels using alpha.
 *
 * @param a Background pixel.
 * @param b Foreground pixel.
 * @return Result of blending B over A.
 */
td_rgba pixel_blend(td_rgba a, td_rgba b);

#endif // TD_TEXTURE_H
