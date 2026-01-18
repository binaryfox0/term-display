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

#include "td_texture.h"

#include "td_def.h"
#include "td_priv.h"
#include "td_rasterizer.h"

#include <stdlib.h>
#include <string.h>

static td_i32 tdp_convert_ch(const td_i32 ch_a, const td_i32 ch_b)
{
    td_bool a_g = IS_GRAYSCALE(ch_a), b_g = IS_GRAYSCALE(ch_b);
    if (!a_g && b_g)
        return ch_b - 2;
    if (a_g && !b_g)
        return ch_b + 2;
    return ch_b;
}

TD_INLINE td_u64 tdp_calculate_size(const td_ivec2 size, const td_i32 channel)
{
    return calculate_pos((td_ivec2){.y = size.y}, size.x, channel);
}

/* Helper utilities end   */

td_texture *td_texture_create(td_u8 *texture,
                             const td_i32 channel,
                             const td_ivec2 size,
                             const td_bool freeable, 
                             const td_bool copy)
{
    if (OUT_RANGE(channel, 1, 4))
        return 0;
    td_texture *out = 0;
    if (!(out = (td_texture *) malloc(sizeof(td_texture))))
        return 0;

    memset(out, 0, sizeof(*out));

    out->size = size;
    out->channel = channel;
    out->freeable = freeable;

    if(!size.x || !size.y)
        return out;

    td_u64 alloc_size = tdp_calculate_size(size, channel);

    if (!texture || copy) {
        out->data = (td_u8 *) malloc(alloc_size);
        if (!out->data) {
            free(out);
            return 0;
        }
        out->freeable = 1;
        if (copy) {
            if (!texture) {
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

td_bool td_texture_set_buffer(td_texture * texture,
                              td_u8* buffer,
                              const td_ivec2 size,
                              const td_i32 channel)
{
    if(!texture)
        return td_false;
    if(OUT_RANGE(channel, 0, 4))
        return td_false;
    if(!buffer)
    {
        td_i32 ch = channel == 0 ? texture->channel : channel;
        if(OUT_RANGE(ch, 1, 4))
            return td_false;
        td_u64 raw_sz = tdp_calculate_size(size, ch);
        td_u8* new_buf = (td_u8*)calloc(1, raw_sz);
        if(!new_buf)
            return td_false;
        if(texture->freeable)
            free(texture->data);
        texture->data = new_buf;
        texture->freeable = td_true;
    } else {
        if(channel == 0)
            return td_false;
        if(texture->freeable)
            free(texture->data);
        texture->data = buffer;
        texture->channel = channel;
    }
    texture->size = size;
    return td_true;
}

td_texture *td_texture_copy(const td_texture *texture)
{
    return td_texture_create(texture->data, texture->channel, texture->size, td_true, td_true);
}

td_u8 *td_texture_get_pixel(const td_texture* texture, const td_ivec2 pos)
{
    if (!texture || pos.x >= texture->size.x || pos.y >= texture->size.y)
        return 0;
    return &(texture->
             data[(pos.y * texture->size.x + pos.x) * texture->channel]);
}

td_ivec2 tdt_get_size(const td_texture *texture)
{
    if (!texture)
        return (td_ivec2){0};
    return texture->size;
}

void td_texture_fill(const td_texture *texture, const td_rgba color)
{
    if (!texture || !color.a)
        return;
    td_u8 c[4] = TD_EXPAND_RGBA(color); 
    td_i32 tmp = 4;
    td_ivec2 size = texture->size;
    td_u8 *data = texture->data;
    td_i32 ch = texture->channel;

    tdp_convert_color(c, c, ch, 4, &tmp);
    if (IS_TRANSPARENT(ch) || color.a != 255) {
        for (td_i32 row = 0; row < size.y; row++) {
            for (td_i32 col = 0; col < size.x; col++, data += ch)
                tdp_blend(data, c, ch, 4);
        }
        return;
    }
    tdp_fill_buffer(data, c, tdp_calculate_size(size, ch), (td_u64)ch);
}

void td_texture_convert(td_texture* texture, td_i32 new_channel)
{
    if (!texture || !texture->data)
        return;

    const td_i32 old_channel = texture->channel;
    const td_u64 pixel_count = (td_u64)texture->size.x * (td_u64)texture->size.y;

    td_u8* old_data = texture->data;
    td_u8* new_data = malloc(pixel_count * (td_u64)new_channel);
    if (!new_data)
        return;

    td_u8* src = old_data;
    td_u8* dst = new_data;

    /* Same grayscale type */
    if (IS_GRAYSCALE(old_channel) == IS_GRAYSCALE(new_channel)) {

        /* Add alpha */
        if (!IS_TRANSPARENT(old_channel) && IS_TRANSPARENT(new_channel)) {
            const td_i32 alpha_index = new_channel - 1;

            for (td_u64 i = 0; i < pixel_count; ++i) {
                memcpy(dst, src, (td_u64)old_channel);
                dst[alpha_index] = 255;

                src += old_channel;
                dst += new_channel;
            }

        /* Remove alpha */
        } else if (IS_TRANSPARENT(old_channel) && !IS_TRANSPARENT(new_channel)) {

            for (td_u64 i = 0; i < pixel_count; ++i) {
                memcpy(dst, src, (td_u64)new_channel);

                src += old_channel;
                dst += new_channel;
            }

        /* Same format, direct copy */
        } else {
            memcpy(dst, src, pixel_count * (td_u64)new_channel);
        }

    /* Different color space */
    } else {
        for (td_u64 i = 0; i < pixel_count; ++i) {
            tdp_convert_color(dst, src, new_channel, old_channel, 0);

            src += old_channel;
            dst += new_channel;
        }
    }

    free(old_data);
    texture->data = new_data;
    texture->channel = new_channel;
}

td_u8 *tdp_texture_crop_raw(const td_u8 * old, const td_i32 channel,
                            const td_ivec2 old_size, const td_ivec2 new_size);

void td_texture_merge(const td_texture *texture_a,
                   const td_texture *texture_b,
                   const td_ivec2 placement_pos,
                   const td_bool replace)
{
    if (!texture_a || !texture_b)
        return;
    if (placement_pos.x > texture_a->size.x ||
        placement_pos.y > texture_a->size.y)
        return;

    td_i32  ch_a = texture_a->channel, ch_b = texture_b->channel,
            new_ch_b = tdp_convert_ch(ch_a, ch_b);

    td_i32 remaining_x = texture_a->size.x - placement_pos.x;
    td_i32 remaining_y = texture_a->size.y - placement_pos.y;
    td_i32 cp_row = min(remaining_y, texture_b->size.y);
    td_i32 cp_col = min(remaining_x, texture_b->size.x);

    td_u8   *ptr_a = texture_a->data + calculate_pos(placement_pos, texture_a->size.x, ch_a), 
            *ptr_b = texture_b->data;

    td_i32 inc_a = (texture_a->size.x - cp_col) * ch_a;
    td_i32 inc_b = (texture_b->size.x - cp_col) * ch_b;

    td_u8 color[4] = {0};
    for(td_i32 row = 0; row < cp_row; row++, ptr_a += inc_a, ptr_b += inc_b)
    {
        for(td_i32 col = 0; col < cp_col; col++, ptr_a += ch_a, ptr_b += ch_b)
        {
            tdp_convert_color(color, ptr_b, texture_a->channel, texture_b->channel, 0);
            if(replace)
                memcpy(ptr_a, color, (td_u64)new_ch_b);
            else
                tdp_blend(ptr_a, color, texture_a->channel, new_ch_b);

        }
    }
}

td_u8* tdp_texture_resize_impl(const td_u8 *old, const td_i32 channel, const td_ivec2 old_size, const td_ivec2 new_size)
{
    float
        x_ratio = (float)(old_size.x - 1) / (float)(new_size.x - 1),
        y_ratio = (float)(old_size.y - 1) / (float)(new_size.y - 1);
    td_u8 *raw =
        (td_u8 *) malloc(calculate_pos((td_ivec2){.y=new_size.y}, new_size.x, channel)),
        *start = raw;
    if (!raw)
        return 0;

    for (td_i32 row = 0; row < new_size.y; row++) {
        float tmp = (float)row * y_ratio;
        td_i32 iyf = tdp_floor(tmp), iyc = tdp_ceil(tmp);
        float ty = tmp - (td_f32)iyf;
        for (td_i32 col = 0; col < new_size.x; col++) {
            tmp = (float)col * x_ratio;
            td_i32 ixf = tdp_floor(tmp), ixc = tdp_ceil(tmp);
            float tx = tmp - (td_f32)ixf;

            td_u64 
                i00 = calculate_pos((td_ivec2){.x=ixf, .y=iyf}, old_size.x, channel),
                i10 = calculate_pos((td_ivec2){.x=ixc, .y=iyf}, old_size.x, channel),
                i01 = calculate_pos((td_ivec2){ixf, iyc}, old_size.x, channel),
                i11 = calculate_pos((td_ivec2){ixc, iyc}, old_size.x, channel);

            for (td_u8 c = 0; c < channel; c++, raw++)
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

td_u8 *tdp_texture_resize_raw(const td_u8 *old, const td_i32 channel, const td_ivec2 old_size,
                      const td_ivec2 new_size)
{
    td_ivec2 size = tdp_ratio_size(old_size, new_size);
    return tdp_texture_resize_impl(old, channel, old_size, size);
}

void tdt_resize(td_texture *texture, const td_ivec2 size)
{
    if (!texture)
        return;
    td_u8 *tmp =
        tdp_texture_resize_raw(texture->data, texture->channel, texture->size,
                       size);
    if (!tmp)
        return;
    free(texture->data);
    texture->data = tmp;
    texture->size = tdp_ratio_size(texture->size, size);
}

td_bool tdt_resize_internal(td_texture *texture,
                           const td_ivec2 new_size)
{
    if (!texture)
        return 0;
    
    size_t size = calculate_pos((td_ivec2){.y=new_size.y}, new_size.x, texture->channel);
    if(size) {
        td_u8 *tmp = (td_u8 *) realloc(texture->data, size);
        if (!tmp)
            return 1;
        texture->data = tmp;
    } else {
        if(texture->data) {
            free(texture->data);
            texture->data = 0;
        }
    }
    texture->size = new_size;
    return 0;
}

td_u8 *tdp_texture_crop_raw(const td_u8 *old, const td_i32 channel, 
                            const td_ivec2 old_size, const td_ivec2 new_size)
{
    td_u8 *raw = 0, *start = 0;
    if (!
        (raw =
         (td_u8 *) malloc(calculate_pos((td_ivec2){.y=new_size.y}, new_size.x, channel))))
        return 0;
    const td_u8 *ptr = old;
    start = raw;
    td_u64 row_length = (td_u64)(new_size.x * channel), old_length =
        (td_u64)(old_size.x * channel);
    for (td_i32 row = 0; row < new_size.y;
         row++, raw += row_length, ptr += old_length)
        memcpy(raw, ptr, row_length);
    return start;
}

void td_texture_crop(td_texture *texture, const td_ivec2 new_size)
{
    if (!texture || new_size.x >= texture->size.x
        || new_size.y >= texture->size.y)
        return;
    td_u8 *tmp =
        tdp_texture_crop_raw(texture->data, texture->channel, texture->size,
                     new_size);
    if (!tmp)
        return;
    free(texture->data);
    texture->data = tmp;
    texture->size = new_size;
}

void td_texture_destroy(td_texture *texture)
{
    if (!texture)
        return;
    if (texture->freeable)
        free(texture->data);
    free(texture);
}

td_rgba td_blend_pixel(const td_rgba a, const td_rgba b)
{
    td_u8 ar[4] = TD_EXPAND_RGBA(a), br[4] = TD_EXPAND_RGBA(b);
    tdp_blend(ar, br, 4, 4);
    return (td_rgba){.raw = {ar[0], ar[1], ar[2], ar[3]}};
}
