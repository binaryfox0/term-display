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
 * @brief Texture manipulation utilities for term-display.
 *
 * This module provides texture creation, modification, drawing, resizing,
 * blending, and memory management functions used by the rendering system.
 */

#ifndef TD_TEXTURE_H
#define TD_TEXTURE_H

#include <td_def.h>
#include <td_error.h>

typedef struct td_texture td_texture_t;

/**
 * @defgroup color_channel Supported Color Channels
 * @brief Supported pixel formats for td_texture.
 *
 * td_texture uses the same channel layout as `stbi_load()` from stb_image.
 *
 * | Channels | Enum Value            | Description                  |
 * |----------|-----------------------|------------------------------|
 * | 1        | TD_TEXTURE_GRAY       | Grayscale                    |
 * | 2        | TD_TEXTURE_GRAY_ALPHA | Grayscale with alpha         |
 * | 3        | TD_TEXTURE_RGB        | Red, green, blue             |
 * | 4        | TD_TEXTURE_RGB_ALPHA  | Red, green, blue, alpha      |
 *
 * `TD_TEXTURE_UNKNOWN` indicates an invalid or uninitialized format.
 */
typedef enum td_texture_type
{
    TD_TEXTURE_UNKNOWN = 0,
    TD_TEXTURE_GRAY,
    TD_TEXTURE_GRAY_ALPHA,
    TD_TEXTURE_RGB,
    TD_TEXTURE_RGB_ALPHA
} td_texture_type_t;

/**
 * @brief Create an empty texture object.
 *
 * @param type Texture pixel format.
 * @return Pointer to a newly allocated texture object.
 *
 * @details
 * Creates a texture with:
 * - no allocated pixel buffer (`texture = NULL`)
 * - size `{0, 0}`
 * - specified pixel format
 * - `freeable = TD_FALSE`
 *
 * This is useful when the pixel buffer will be assigned later or when a
 * placeholder texture is needed.
 */
td_texture_t *td_texture_create_empty(const td_texture_type_t type);

/**
 * @brief Create a texture object.
 *
 * @param texture Raw 8-bit pixel buffer.
 * @param type Texture pixel format.
 * @param size Texture dimensions in pixels.
 * @param freeable Whether `td_texture_destroy()` should free `texture`.
 * @param copy Whether the pixel data should be copied internally.
 * @return Pointer to a newly created texture object.
 *
 * @details
 * Behavior depends on `texture` and `copy`:
 *
 * - If `texture == NULL`, a zero-initialized buffer is allocated.
 * - If `copy == TD_TRUE`, pixel data is copied into internal storage.
 * - If `freeable == TD_TRUE`, ownership of `texture` is transferred.
 *
 * If either `size.x` or `size.y` is zero, an empty texture object is created
 * without allocating pixel memory.
 */
td_texture_t *td_texture_create(td_u8 *texture,
                                const td_texture_type_t type,
                                const td_ivec2 size,
                                const td_bool freeable,
                                const td_bool copy);

td_error_t td_texture_paramater(void);

/**
 * @brief Replace the internal pixel buffer of a texture.
 *
 * @param texture Target texture.
 * @param buffer Raw 8-bit pixel buffer.
 * @param size New texture dimensions.
 * @param type New texture pixel format.
 * @return Operation result.
 *
 * @details
 * Replaces the internal pixel buffer of an existing texture.
 *
 * - If `buffer == NULL`, a new zero-initialized buffer is allocated.
 * - If `type == TD_TEXTURE_UNKNOWN`, the current texture format is preserved.
 * - If `buffer != NULL`, ownership and memory behavior follow the settings
 *   defined when the texture was originally created.
 */
td_error_t td_texture_set_buffer(td_texture_t *texture,
                              td_u8 *buffer,
                              const td_ivec2 size,
                              const td_texture_type_t type);

/**
 * @brief Create a deep copy of a texture.
 *
 * @param texture Source texture.
 * @return Newly allocated duplicate texture.
 */
td_texture_t *td_texture_copy(const td_texture_t *texture);

/**
 * @brief Get the address of a pixel.
 *
 * @param texture Texture to query.
 * @param pos Pixel position.
 * @return Pointer to the pixel data at `pos`.
 *
 * @details
 * Returned pixel layout depends on the texture format.
 */
td_u8 *td_texture_get_pixel(const td_texture_t *texture,
                            const td_ivec2 pos);

/**
 * @brief Get texture dimensions.
 *
 * @param texture Texture to query.
 * @return Width and height as `td_ivec2`.
 */
td_ivec2 td_texture_get_size(const td_texture_t *texture);

/**
 * @brief Fill a texture with a single color.
 *
 * @param texture Target texture.
 * @param color Fill color.
 * @return Operation result.
 *
 * @details
 * Writes `color` across the entire texture buffer. Alpha blending is applied
 * when supported by the texture format.
 */
td_error_t td_texture_fill(const td_texture_t *texture,
                           const td_rgba color);

/**
 * @brief Resize a texture buffer without resampling pixel data.
 *
 * Reallocates the internal texture buffer to match the requested dimensions.
 * Existing pixel data is preserved only within the overlapping region between
 * the old and new sizes. Newly allocated regions will be zero'ed by default
 *
 * Unlike td_texture_resize(), this function does not perform bilinear
 * interpolation or any image scaling operation.
 *
 * @param texture Texture whose buffer will be resized.
 * @param new_size Target buffer dimensions.
 * @return Operation result.
 */
td_error_t td_texture_resize_buffer(td_texture_t *texture,
                                    const td_ivec2 new_size);

/**
 * @brief Destroy a texture object.
 *
 * @param texture Texture to destroy.
 * @return Operation result.
 *
 * @details
 * Releases the texture object and its internal pixel buffer when owned by
 * the texture.
 */
td_error_t td_texture_destroy(td_texture_t *texture);

/**
 * @brief Alpha-blend two pixels.
 *
 * @param a Background pixel.
 * @param b Foreground pixel.
 * @return Result of blending `b` over `a`.
 */
td_rgba td_blend_color(const td_rgba a,
                       const td_rgba b);

#endif // TD_TEXTURE_H
