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

static td_i32 tdp_convert_ch(const td_i32 ch_a, const td_i32 ch_b)
{
    td_bool a_g = TDP_IS_GRAY(ch_a), b_g = TDP_IS_GRAY(ch_b);
    if (!a_g && b_g)
        return ch_b - 2;
    if (a_g && !b_g)
        return ch_b + 2;
    return ch_b;
}

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

    if(OUT_RANGE(type, 1, 4) || size.x < 0 || size.y < 0)
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
        OUT_RANGE(type, 1, 4) ||
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

td_error_t td_texture_convert(td_texture_t *texture,
                              const td_texture_type_t type)
{
    td_texture_type_t old_type = TD_TEXTURE_UNKNOWN; 
    td_u8* old_data = 0;
    td_u64 pixel_count = 0;
    td_u8 *new_data = 0;
    td_u8 *src = 0;
    td_u8 *dst = 0;
    if (!texture || OUT_RANGE(type, 1, 4))
        return TD_ERR_INVALID_ARG;

    old_data = texture->data;
    pixel_count = (td_u64)texture->size.x * (td_u64)texture->size.y;
    new_data = malloc(pixel_count * (td_u64)type);
    if (!new_data)
        return TD_ERR_OUT_OF_MEMORY;

    old_type = texture->type;
    src = old_data;
    dst = new_data;

    /* Same grayscale type */
    if (TDP_IS_GRAY(old_type) == TDP_IS_GRAY(type)) 
    {
        /* Add alpha */
        if (!TDP_HAS_ALPHA(old_type) && TDP_HAS_ALPHA(type)) 
        {
            const td_i32 alpha_index = (td_i32)type - 1;

            for (td_u64 i = 0; i < pixel_count; ++i) {
                memcpy(dst, src, (size_t)old_type);
                dst[alpha_index] = 255;

                src += old_type;
                dst += type;
            }

        /* Remove alpha */
        } else if (TDP_HAS_ALPHA(old_type) && !TDP_HAS_ALPHA(type)) 
        {
            for (td_u64 i = 0; i < pixel_count; ++i) {
                memcpy(dst, src, (size_t)type);

                src += old_type;
                dst += type;
            }

        /* Same format, direct copy */
        } else {
            memcpy(dst, src, pixel_count * (td_u64)type);
        }

    /* Different color space */
    } else {
        for (td_u64 i = 0; i < pixel_count; ++i) {
            tdp_convert_color(dst, src, (td_i32)type, 
                    (td_i32)old_type, 0);

            src += old_type;
            dst += type;
        }
    }

    free(old_data);
    texture->data = new_data;
    texture->type = type;
    return TD_ERR_OK;
}

td_u8 *tdp_texture_crop_impl(const td_u8 * old, const td_i32 ch,
                            const td_ivec2 old_size, const td_ivec2 new_size);

td_error_t td_texture_merge(const td_texture_t *texture_a,
                            const td_texture_t *texture_b,
                            const td_irect src_rect,
                            const td_irect dst_rect,
                            const td_texture_merge_mode_t mode)
{
    td_i32 ch_a = 0;
    td_i32 ch_b = 0;
    td_i32 new_ch_b = 0;

    td_irect nsrc = {0};
    td_irect ndst = {0};

    td_i32 src_x = 0;
    td_i32 src_y = 0;
    td_i32 dst_x = 0;
    td_i32 dst_y = 0;

    td_i32 cp_w = 0;
    td_i32 cp_h = 0;

    td_u8 color[4] = {0};

    if (!texture_a || !texture_b)
        return TD_ERR_INVALID_ARG;

    ch_a = (td_i32)texture_a->type;
    ch_b = (td_i32)texture_b->type;
    new_ch_b = tdp_convert_ch(ch_a, ch_b);

    nsrc = src_rect;
    ndst = dst_rect;

    if (nsrc.w < 0) nsrc.w = texture_b->size.x - nsrc.x;
    if (nsrc.h < 0) nsrc.h = texture_b->size.y - nsrc.y;

    if (ndst.w < 0) ndst.w = texture_a->size.x - ndst.x;
    if (ndst.h < 0) ndst.h = texture_a->size.y - ndst.y;

    src_x = nsrc.x;
    src_y = nsrc.y;
    dst_x = ndst.x;
    dst_y = ndst.y;

    if (dst_x >= texture_a->size.x || dst_y >= texture_a->size.y)
        return TD_ERR_OK;

    if (mode != TD_TEXTURE_MERGE_WRAP)
    {
        if (src_x >= texture_b->size.x || src_y >= texture_b->size.y)
            return TD_ERR_OK;
    }

    cp_w = tdp_min(nsrc.w, ndst.w);
    cp_h = tdp_min(nsrc.h, ndst.h);

    if (mode != TD_TEXTURE_MERGE_WRAP)
    {
        cp_w = tdp_min(cp_w, texture_b->size.x - src_x);
        cp_h = tdp_min(cp_h, texture_b->size.y - src_y);
    }

    cp_w = tdp_min(cp_w, texture_a->size.x - dst_x);
    cp_h = tdp_min(cp_h, texture_a->size.y - dst_y);

    if (cp_w <= 0 || cp_h <= 0)
        return TD_ERR_OK;

    td_i32 inc_a = (texture_a->size.x - cp_w) * ch_a;

    td_u8 *ptr_a = texture_a->data +
        tdp_calculate_pos((td_ivec2){.x=dst_x, .y=dst_y},
                          texture_a->size.x,
                          ch_a);

    for (td_i32 row = 0; row < cp_h; row++, ptr_a += inc_a)
    {
        td_u8 *row_a = ptr_a;

        for (td_i32 col = 0; col < cp_w; col++, row_a += ch_a)
        {
            td_i32 sx = src_x + col;
            td_i32 sy = src_y + row;

            td_u8 *ptr_b = 0;

            if (mode == TD_TEXTURE_MERGE_WRAP)
            {
                sx = (sx % texture_b->size.x + texture_b->size.x) % texture_b->size.x;
                sy = (sy % texture_b->size.y + texture_b->size.y) % texture_b->size.y;
            }

            ptr_b = texture_b->data +
                tdp_calculate_pos((td_ivec2){.x=sx, .y=sy},
                                  texture_b->size.x,
                                  ch_b);

            tdp_convert_color(color, ptr_b, ch_a, ch_b, 0);

            switch (mode)
            {
                case TD_TEXTURE_MERGE_REPLACE:
                    memcpy(row_a, color, (size_t)ch_a);
                    break;

                case TD_TEXTURE_MERGE_BLEND:
                case TD_TEXTURE_MERGE_WRAP:
                    tdp_blend(row_a, color, ch_a, new_ch_b, row_a);
                    break;
            }
        }
    }

    return TD_ERR_OK;
}

td_u8* tdp_texture_resize_impl(
        const td_u8 *old, 
        const td_i32 ch, 
        const td_ivec2 old_size, 
        const td_ivec2 new_size)
{
    float x_ratio = 0.0f;
    float y_ratio = 0.0f;
    td_u8 *raw = 0;
    td_u8 *start = 0;
        
    float tmp = 0.0f;
    td_i32 iyf = 0;
    td_i32 iyc = 0;
    float ty = 0.0f;

    td_i32 ixf = 0;
    td_i32 ixc = 0;
    float tx = 0.0f;

    td_u64 i00 = 0;
    td_u64 i10 = 0;
    td_u64 i01 = 0;
    td_u64 i11 = 0;
        
    x_ratio = (float)(old_size.x - 1) / (float)(new_size.x - 1),
    y_ratio = (float)(old_size.y - 1) / (float)(new_size.y - 1);
    raw = malloc(tdp_calculate_pos((td_ivec2){.y=new_size.y}, 
                new_size.x, ch));
    if (!raw)
        return 0;
    start = raw;

    for (td_i32 row = 0; row < new_size.y; row++) 
    {
        tmp = (float)row * y_ratio;
        iyf = tdp_floor(tmp);
        iyc = tdp_ceil(tmp);
        ty = tmp - (td_f32)iyf;
        for (td_i32 col = 0; col < new_size.x; col++) 
        {
            tmp = (float)col * x_ratio;
            ixf = tdp_floor(tmp);
            ixc = tdp_ceil(tmp);
            tx = tmp - (td_f32)ixf;

            i00 = tdp_calculate_pos((td_ivec2){.x = ixf, .y = iyf}, 
                    old_size.x, ch),
            i10 = tdp_calculate_pos((td_ivec2){.x = ixc, .y = iyf}, 
                    old_size.x, ch),
            i01 = tdp_calculate_pos((td_ivec2){.x = ixf, .y = iyc}, 
                    old_size.x, ch),
            i11 = tdp_calculate_pos((td_ivec2){.x = ixc, .y = iyc}, 
                    old_size.x, ch);

            for (td_u32 c = 0; c < (td_u32)ch; c++, raw++)
                raw[0] =
                    bilerp(old[i00 + c], old[i10 + c], old[i01 + c],
                           old[i11 + c], tx, ty);
        }
    }
    return start;
}


td_ivec2 tdp_ratio_size(const td_ivec2 old, const td_ivec2 size)
{
    if (!size.x)
        return (td_ivec2){.x=(old.x * size.y) / old.y, .y=size.y};
    if (!size.y)
        return (td_ivec2){.x=size.x, .y=(old.y * size.x) / old.x};
    return size;
}

td_error_t td_texture_resize(td_texture_t *texture,
                             const td_ivec2 new_size)
{
    td_ivec2 ratio_size = {0};
    td_u8 *tmp = 0;
    if (
            !texture ||
            new_size.x < 0 || new_size.y < 0 ||
            (new_size.x == 0 && new_size.y == 0)) 
        return TD_ERR_INVALID_ARG;

    if(texture->size.x == new_size.x && 
            texture->size.y == new_size.y)
        return TD_ERR_OK;

    ratio_size = tdp_ratio_size(texture->size, new_size);
    tmp = tdp_texture_resize_impl(texture->data, (int)texture->type, 
            texture->size, ratio_size);
    if (!tmp)
        return TD_ERR_OUT_OF_MEMORY;

    free(texture->data);
    texture->data = tmp;
    texture->size = tdp_ratio_size(texture->size, ratio_size);
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

td_u8 *tdp_texture_crop_impl(const td_u8 *old, const td_i32 ch, 
                            const td_ivec2 old_size, const td_ivec2 new_size)
{
    td_u8 *raw = 0, *start = 0;
    raw = malloc(tdp_calculate_pos((td_ivec2){.y=new_size.y},
                new_size.x, ch));
        return 0;
    const td_u8 *ptr = old;
    start = raw;
    td_u64 row_length = (td_u64)(new_size.x * ch), old_length =
        (td_u64)(old_size.x * ch);
    for (td_i32 row = 0; row < new_size.y;
         row++, raw += row_length, ptr += old_length)
        memcpy(raw, ptr, row_length);
    return start;
}

td_error_t td_texture_crop(
        td_texture_t *texture, 
        const td_ivec2 new_size)
{
    td_u8 *tmp = 0;
    if (!texture || 
        new_size.x >= texture->size.x || 
        new_size.y >= texture->size.y)
        return TD_ERR_INVALID_ARG;
        
    tmp = tdp_texture_crop_impl(texture->data, (td_i32)texture->type, 
            texture->size, new_size);
    if (!tmp)
        return TD_ERR_OUT_OF_MEMORY;
    free(texture->data);
    texture->data = tmp;
    texture->size = new_size;
    return TD_ERR_OK;
}

td_error_t td_texture_destroy(td_texture_t *texture)
{
    if (!texture)
        return TD_ERR_INVALID_ARG;
    if(texture->lib_owned)
        return TD_ERR_FORBIDDEN;
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

