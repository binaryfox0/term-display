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

#include "td_priv.h"
#include "td_rasterizer.h"

#include <stdlib.h>
#include <string.h>

td_u8 convert_ch(td_u8 ch_a, td_u8 ch_b)
{
    td_u8 a_g = IS_GRAYSCALE(ch_a), b_g = IS_GRAYSCALE(ch_b);
    if (!a_g && b_g)
        return ch_b - 2;
    if (a_g && !b_g)
        return ch_b + 2;
    return ch_b;
}

/* Helper utilities end   */

td_texture *tdt_create(td_u8 *texture,
                             const td_u8 channel,
                             const td_ivec2 size,
                             const td_u8 freeable, const td_u8 copy)
{
    if (OUT_RANGE(channel, 1, 4))
        return 0;
    td_texture *out = 0;
    if (!(out = (td_texture *) malloc(sizeof(td_texture))))
        return 0;

    out->size = size;
    out->channel = channel;

    if(!size.x || !size.y)
        return out;

    td_u64 alloc_size = calculate_pos(0, size.y, size.x, channel);

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
        out->freeable = freeable;
    }

    return out;
}

td_texture *tdt_copy(td_texture *texture)
{
    td_texture *out = 0;
    if (!(out = (td_texture *) malloc(sizeof(td_texture))))
        return 0;
    td_u64 size = calculate_size(texture->size.x, texture->size.y, texture->channel);
    if (!(out->data = (td_u8 *) malloc(size))) {
        free(out);
        return 0;
    }
    memcpy(out->data, texture->data, size);
    out->freeable = 1;
    out->size = texture->size;
    out->channel = texture->channel;
    return out;
}

td_u8 *tdt_get_location(const td_ivec2 pos, const td_texture *texture)
{
    if (!texture || pos.x >= texture->size.x || pos.y >= texture->size.y)
        return 0;
    return &(texture->
             data[(pos.y * texture->size.x + pos.x) * texture->channel]);
}

td_ivec2 tdt_get_size(const td_texture *texture)
{
    if (!texture)
        return td_ivec2_init(0, 0);
    return texture->size;
}

void tdt_fill(const td_texture *texture, const td_rgba color)
{
    if (!texture || !color.a)
        return;
    td_u8 c[4] = TD_EXPAND_RGBA(color), tmp = 4;
    td_ivec2 size = texture->size;
    td_u8 *data = texture->data;
    td_u8 ch = texture->channel;

    convert(c, c, ch, 4, &tmp);
    if (IS_TRANSPARENT(ch) || color.a != 255) {
        for (td_i32 row = 0; row < size.y; row++) {
            for (td_i32 col = 0; col < size.x; col++, data += ch)
                alpha_blend(data, c, ch, 4);
        }
        return;
    }
    fill_buffer(data, c, calculate_size(size.x, size.y, ch), ch);
}

void tdt_set_channel(td_texture* texture, td_u8 channel)
{
    td_u64 pix_count = calculate_size(texture->size.x, texture->size.y, 1);

    td_u8* old_ptr = texture->data;
    td_u8* tmp = (td_u8*)malloc(calculate_size(texture->size.x, texture->size.y, channel));
    if(!tmp) return;
    texture->data = tmp;
    td_u8 old_channel = texture->channel;
    texture->channel = channel;

    if(IS_GRAYSCALE(texture->channel) == IS_GRAYSCALE(channel)) // Same type
    {
        if(!IS_TRANSPARENT(texture->channel) && IS_TRANSPARENT(channel)) {
            td_u8 a_i = channel - 1;
            for(td_u64 i = 0; i < pix_count; i++, old_ptr += old_channel, tmp += channel) {
                memcpy(tmp, old_ptr, old_channel);
                tmp[a_i] = 255;
            }
        } else if (IS_TRANSPARENT(texture->channel) && !IS_TRANSPARENT(channel)) {
            for(td_u64 i = 0; i < pix_count; i++, old_ptr += old_channel, tmp += channel)
                memcpy(tmp, old_ptr, channel);
        }
    } else {
        for(td_u64 i = 0; i < pix_count; i++, old_ptr += old_channel, tmp += channel)
            convert(tmp, old_ptr, channel, old_channel, 0);
    }
    free(old_ptr);
}

// Forward declaration section
td_u8 *crop_texture(td_u8 * old, td_u8 channel, td_ivec2 old_size,
                 td_ivec2 new_size);
td_u8 *resize_texture(const td_u8 * old, td_u8 channel, td_ivec2 old_size,
                   td_ivec2 new_size);

void tdt_merge(const td_texture *texture_a,
                   const td_texture *texture_b,
                   const td_ivec2 placement_pos,
                   const enum tdt_merge_mode mode, const td_bool replace)
{
    if (!texture_a || !texture_b)
        return;
    if (placement_pos.x > texture_a->size.x ||
        placement_pos.y > texture_a->size.y)
        return;
    td_u8 cha = texture_a->channel, chb = texture_b->channel;
    td_ivec2 sa = texture_a->size, sb = texture_b->size;
    td_u8 *ta =
        &texture_a->data[(placement_pos.y * sa.x + placement_pos.x) * cha],
        *tb = texture_b->data, *old = 0;

    // Apply size thersehold
    td_i32 max_space_x = sa.x - placement_pos.x, max_space_y =
        sa.y - placement_pos.y;
    td_u8 b_freeable = sb.x > max_space_x || sb.y > max_space_y;
    if (b_freeable) {
        td_ivec2 new_size =
            td_ivec2_init(sb.x > max_space_x ? max_space_x : sb.x,
                       sb.y > max_space_y ? max_space_y : sb.y);
        if (mode == TDT_MERGE_RESIZE)
            tb = resize_texture(tb, chb, sb, new_size);
        else
            tb = crop_texture(tb, chb, sb, new_size);
        old = tb;
        sb = new_size;
    }

    td_i32 space = (sa.x - sb.x) * cha;
    for (td_i32 row = 0; row < sb.y; row++, ta += space) {
        for (td_i32 col = 0; col < sb.x; col++, ta += cha, tb += chb) {
            td_u8 tmp[4] = { 0 }, tmp_1 = chb;
            convert(tmp, tb, cha, chb, &tmp_1);
            if(replace) 
                memcpy(ta, tmp, cha);
            else 
                alpha_blend(ta, tmp, cha, tmp_1);
        }
    }
    if (b_freeable)
        free(old);
}

td_ivec2 calculate_new_size(const td_ivec2 old, const td_ivec2 size)
{
    if (!size.x)
        return td_ivec2_init((old.x * size.y) / old.y, size.y);
    if (!size.y)
        return td_ivec2_init(size.x, (old.y * size.x) / old.x);
    return size;
}

td_u8 *resize_texture(const td_u8 *old, td_u8 channel, td_ivec2 old_size,
                   td_ivec2 new_size)
{
    if (!new_size.x || !new_size.y)
        new_size = calculate_new_size(old_size, new_size);
    return ptexture_resize(old, channel, old_size, new_size);
}

void tdt_resize(td_texture *texture, const td_ivec2 size)
{
    if (!texture)
        return;
    td_u8 *tmp =
        resize_texture(texture->data, texture->channel, texture->size,
                       size);
    if (!tmp)
        return;
    free(texture->data);
    texture->data = tmp;
    texture->size = calculate_new_size(texture->size, size);
}

td_bool tdt_resize_internal(td_texture *texture,
                           const td_ivec2 new_size)
{
    if (!texture)
        return 0;
    
    size_t size = calculate_size(new_size.x, new_size.y, texture->channel);
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

td_u8 *crop_texture(td_u8 *old, td_u8 channel, td_ivec2 old_size,
                 td_ivec2 new_size)
{
    td_u8 *raw = 0;
    if (!
        (raw =
         (td_u8 *) malloc(calculate_pos(0, new_size.y, new_size.x, channel))))
        return 0;
    td_u8 *ptr = old, *start = raw;
    td_u64 row_length = (td_u64)(new_size.x * channel), old_length =
        (td_u64)(old_size.x * channel);
    for (td_i32 row = 0; row < new_size.y;
         row++, raw += row_length, ptr += old_length)
        memcpy(raw, ptr, row_length);
    return start;
}

void tdt_crop(td_texture *texture, const td_ivec2 new_size)
{
    if (!texture || new_size.x >= texture->size.x
        || new_size.y >= texture->size.y)
        return;
    td_u8 *tmp =
        crop_texture(texture->data, texture->channel, texture->size,
                     new_size);
    if (!tmp)
        return;
    free(texture->data);
    texture->data = tmp;
    texture->size = new_size;
}

// void tdt_draw_line(td_texture *texture, const td_ivec2 p1,
                    //    const td_ivec2 p2, const td_rgba color)
// {
    // ptexture_draw_line((td_framebuffer){texture->data, texture->channel, 0, texture->size},
                    //    p1, p2, td_vec2_init(0.0f, 0.0f), color);
// }

void tdt_free(td_texture *texture)
{
    if (!texture)
        return;
    if (texture->freeable)
        free(texture->data);
    free(texture);
}

td_rgba pixel_blend(td_rgba a, td_rgba b)
{
    td_u8 ar[4] = TD_EXPAND_RGBA(a), br[4] = TD_EXPAND_RGBA(b);
    alpha_blend(ar, br, 4, 4);
    return td_rgba_init(ar[0], ar[1], ar[2], ar[3]);
}
