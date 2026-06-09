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

#include <td_texture.h>
#include "td_texture_priv.h"

#include <td_def.h>
#include "td_utils.h"
#include "td_rasterizer.h"

#include <stdlib.h>
#include <string.h>

/*
static td_i32 tdp_convert_ch(const td_i32 ch_a, const td_i32 ch_b)
{
    td_bool a_g = TDP_IS_GRAY(ch_a), b_g = TDP_IS_GRAY(ch_b);
    if (!a_g && b_g)
        return ch_b - 2;
    if (a_g && !b_g)
        return ch_b + 2;
    return ch_b;
}
*/
/* Helper utilities end   */
td_texture_t *td_texture_create_empty(const td_texture_type_t type)
{
    td_texture_t *out = 0;
    out = calloc(1, sizeof(*out));
    if(!out)
        return 0;
    out->freeable = TD_TRUE;
    out->type = type;
    return out;
}

td_texture_t *td_texture_create(td_u8 *texture,
                             const td_texture_type_t type,
                             const td_ivec2 size,
                             const td_bool freeable, 
                             const td_bool copy)
{
    td_texture_t *out = 0;
    td_u64 alloc_size = 0;

    if(TDP_OUT_RANGE(type, 1, 4) || size.x < 0 || size.y < 0)
        return 0;

    out = calloc(1, sizeof(*out));
    if(!out)
        return 0;

    out->type = type;
    out->freeable = freeable;
    if(!size.x || !size.y)
        return out;
    out->size = size;

    alloc_size = tdp_calculate_size(size, (td_i32)type);
    if (!texture || copy) 
    {
        out->data = (td_u8 *) malloc(alloc_size);
        if (!out->data) {
            free(out);
            return 0;
        }
        out->freeable = TD_TRUE;
        if (copy) 
        {
            if (!texture) 
            {
                free(out->data);
                free(out);
                return 0;
            }
            memcpy(out->data, texture, alloc_size);
            if (freeable)
                free(texture);
        } else if (!texture)
            memset(out->data, 0, alloc_size);
    } else {
        out->data = texture;
    }

    return out;
}

td_error_t td_texture_set_buffer(td_texture_t *texture,
                              td_u8 *buffer,
                              const td_ivec2 size,
                              const td_texture_type_t type)
{
    if(
        !texture ||
        TDP_OUT_RANGE(type, 1, 4) ||
        size.x < 0 || size.y < 0
      ) return TD_ERR_INVALID_ARG;

    if(!buffer)
    {
        td_u64 raw_sz = 0;
        td_u8* new_buf = 0;

        raw_sz = tdp_calculate_size(size, (int)type);
        new_buf = calloc(1, raw_sz);
        if(!new_buf)
            return TD_ERR_OUT_OF_MEMORY;
        if(texture->freeable)
            free(texture->data);
        texture->data = new_buf;
        texture->freeable = TD_TRUE;
    } else {
        if(texture->freeable)
            free(texture->data);
        texture->data = buffer;
        texture->type = type;
    }
    texture->size = size;
    return TD_ERR_OK;
}

td_texture_t *td_texture_copy(const td_texture_t *texture)
{
    return td_texture_create(
            texture->data, 
            texture->type, 
            texture->size, 
            TD_TRUE, 
            TD_TRUE);
}

td_u8 *td_texture_get_pixel(const td_texture_t* texture, const td_ivec2 pos)
{
    if (
            !texture || 
            !texture->data ||
            pos.x >= texture->size.x || 
            pos.y >= texture->size.y)
        return 0;
    
    return texture->data +
        tdp_calculate_pos(pos, texture->size.x, (td_i32)texture->type);
}

td_ivec2 td_texture_get_size(const td_texture_t *texture)
{
    if (!texture)
        return (td_ivec2){0};
    return texture->size;
}

td_error_t td_texture_fill(const td_texture_t *texture, const td_rgba color)
{
    td_u8 c[4] = {0}; 
    td_ivec2 size = texture->size;
    td_u8 *data = texture->data;
    td_i32 ch = (td_i32)texture->type;

    if (!texture)
        return TD_ERR_INVALID_ARG;
    if(!texture->data || color.a == 0)
        return TD_ERR_OK;

    tdp_convert_color(c, color.raw, ch, 4, 0);
    if (TDP_HAS_ALPHA(ch) || color.a != 255) {
        for (td_i32 row = 0; row < size.y; row++) {
            for (td_i32 col = 0; col < size.x; col++, data += ch)
                tdp_blend(data, c, ch, 4, data);
        }
    } else {
        tdp_fill_buffer(data, c, tdp_calculate_size(size, ch), (td_u64)ch);    
    }
    return TD_ERR_OK;
}

td_error_t td_texture_resize_buffer(td_texture_t *texture,
                           const td_ivec2 new_size)
{
    size_t old_alloc_size = 0;
    size_t alloc_size = 0;
    if (
            !texture ||
            new_size.x < 0 ||
            new_size.y < 0)
        return TD_ERR_INVALID_ARG;
   
    if(texture->size.x == new_size.x && 
            texture->size.y == new_size.y)
        return TD_ERR_OK;

    old_alloc_size = tdp_calculate_size(texture->size, (td_i32)texture->type);
    alloc_size = tdp_calculate_size(new_size, (td_i32)texture->type);
    if(alloc_size) 
    {
        td_u8 *tmp = 0;
        tmp = realloc(texture->data, alloc_size);
        if (!tmp)
            return TD_ERR_OUT_OF_MEMORY;

        if(alloc_size > old_alloc_size)
            memset(tmp + old_alloc_size, 0, alloc_size - old_alloc_size);

        texture->data = tmp;
        texture->size = new_size;
    } else {
        if(texture->data) {
            free(texture->data);
            texture->data = 0;
        }
        texture->size = (td_ivec2){0};
    }
    return TD_ERR_OK;
}

td_error_t td_texture_destroy(td_texture_t *texture)
{
    if (!texture)
        return TD_ERR_INVALID_ARG;
    if (texture->freeable)
        free(texture->data);
    free(texture);
    return TD_ERR_OK;
}

td_rgba td_blend_color(const td_rgba a,
                       const td_rgba b)
{
    td_rgba out = {0};
    tdp_blend(a.raw, b.raw, 4, 4, out.raw);
    return out;
}

void tdp_blend(
        const td_u8 *a, 
        const td_u8 *b, 
        const td_i32 ch_a, 
        const td_i32 ch_b,
        td_u8 *dst)
{
    td_bool has_alpha = TD_FALSE;
    td_i32 alpha_idx = 0;
    td_i32 a_alpha = 0, b_alpha = 0;
    td_i32 inv_b_alpha = 0;

    has_alpha = TDP_HAS_ALPHA(ch_a);
    alpha_idx = ch_a - 1;
    a_alpha = has_alpha ? a[alpha_idx] : 255;
    b_alpha = TDP_HAS_ALPHA(ch_b) ? b[ch_b - 1] : 255;
    inv_b_alpha = 255 - b_alpha;

    if (ch_a < 5)
        dst[0] = (td_u8)((b_alpha * b[0] + inv_b_alpha * a[0]) >> 8);
    if (ch_a > 2) 
    {
        dst[1] = (td_u8)((b_alpha * b[1] + inv_b_alpha * a[1]) >> 8);
        dst[2] = (td_u8)((b_alpha * b[2] + inv_b_alpha * a[2]) >> 8);
    }
    if (has_alpha)
        dst[alpha_idx] = (td_u8)(!inv_b_alpha ? 255 : 
                b_alpha + ((inv_b_alpha + a_alpha) >> 8));
}

// // Convert b to have the same type as a
void tdp_convert_color(
        td_u8 *dst, const td_u8 *src,
        const td_i32 src_ch, 
        const td_i32 target_ch,
        td_i32 *conv_ch)
{
    td_bool src_gray = TDP_IS_GRAY(src_ch);
    td_bool dst_gray = TDP_IS_GRAY(target_ch);

    if (src_gray && !dst_gray) 
    {
        // Grayscale -> Truecolor
        dst[0] = to_grayscale(src);
        dst[1] = target_ch - 3 ? src[3] : 255;

        if(conv_ch)
            *conv_ch = target_ch - 2;
        return;
    }

    if (!src_gray && dst_gray) 
    {
        // Truecolor -> Grayscale
        dst[0] = dst[1] = dst[2] = src[0];
        dst[3] = target_ch - 1 ? src[1] : 255;

        if(conv_ch)
            *conv_ch = target_ch + 2;
        return;
    }

    // Same format, direct copy
    for (td_i32 i = 0; i < target_ch; i++)
        dst[i] = src[i];

    if(conv_ch)
        *conv_ch = target_ch;
}

