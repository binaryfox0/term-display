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

#include "term_priv.h"
#include "term_texture_priv.h"

#include <stdlib.h>
#include <string.h>

struct term_texture_s {
    term_u8 *data;
    term_u8 channel;
    term_ivec2 size;
    term_u8 freeable;
};

term_u8 convert_ch(term_u8 ch_a, term_u8 ch_b)
{
    term_u8 a_g = IS_GRAYSCALE(ch_a), b_g = IS_GRAYSCALE(ch_b);
    if (!a_g && b_g)
        return ch_b - 2;
    if (a_g && !b_g)
        return ch_b + 2;
    return ch_b;
}

/* Helper utilities end   */

term_texture *tdt_create(term_u8 *texture,
                             const term_u8 channel,
                             const term_ivec2 size,
                             const term_u8 freeable, const term_u8 copy)
{
    if (OUT_RANGE(channel, 1, 4) || !size.x || !size.y)
        return 0;
    term_texture *out = 0;
    if (!(out = (term_texture *) malloc(sizeof(term_texture))))
        return 0;
    term_u64 alloc_size = calculate_pos(0, size.y, size.x, channel);

    if (!texture || copy) {
        out->data = (term_u8 *) malloc(alloc_size);
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

    out->size = size;
    out->channel = channel;
    return out;
}

term_texture *tdt_copy(term_texture *texture)
{
    term_texture *out = 0;
    if (!(out = (term_texture *) malloc(sizeof(term_texture))))
        return 0;
    term_u64 size = calculate_size(texture->size.x, texture->size.y, texture->channel);
    if (!(out->data = (term_u8 *) malloc(size))) {
        free(out);
        return 0;
    }
    memcpy(out->data, texture->data, size);
    out->freeable = 1;
    out->size = texture->size;
    out->channel = texture->channel;
    return out;
}

term_u8 *tdt_get_location(const term_ivec2 pos, const term_texture *texture)
{
    if (!texture || pos.x >= texture->size.x || pos.y >= texture->size.y)
        return 0;
    return &(texture->
             data[(pos.y * texture->size.x + pos.x) * texture->channel]);
}

term_ivec2 tdt_get_size(const term_texture *texture)
{
    if (!texture)
        return ivec2_init(0, 0);
    return texture->size;
}

void tdt_fill(const term_texture *texture, const term_rgba color)
{
    if (!texture || !color.a)
        return;
    term_u8 c[4] = EXPAND_RGBA(color), tmp = 4;
    term_ivec2 size = texture->size;
    term_u8 *data = texture->data;
    term_u8 ch = texture->channel;

    convert(c, c, ch, 4, &tmp);
    if (IS_TRANSPARENT(ch) || color.a != 255) {
        for (term_i32 row = 0; row < size.y; row++) {
            for (term_i32 col = 0; col < size.x; col++, data += ch)
                alpha_blend(data, c, ch, 4);
        }
        return;
    }
    fill_buffer(data, c, calculate_size(size.x, size.y, ch), ch);
}

void tdt_set_channel(term_texture* texture, term_u8 channel)
{
    term_u64 pix_count = calculate_size(texture->size.x, texture->size.y, 1);

    term_u8* old_ptr = texture->data;
    term_u8* tmp = (term_u8*)malloc(calculate_size(texture->size.x, texture->size.y, channel));
    if(!tmp) return;
    texture->data = tmp;
    term_u8 old_channel = texture->channel;
    texture->channel = channel;

    if(IS_GRAYSCALE(texture->channel) == IS_GRAYSCALE(channel)) // Same type
    {
        if(!IS_TRANSPARENT(texture->channel) && IS_TRANSPARENT(channel)) {
            term_u8 a_i = channel - 1;
            for(term_u64 i = 0; i < pix_count; i++, old_ptr += old_channel, tmp += channel) {
                memcpy(tmp, old_ptr, old_channel);
                tmp[a_i] = 255;
            }
        } else if (IS_TRANSPARENT(texture->channel) && !IS_TRANSPARENT(channel)) {
            for(term_u64 i = 0; i < pix_count; i++, old_ptr += old_channel, tmp += channel)
                memcpy(tmp, old_ptr, channel);
        }
    } else {
        for(term_u64 i = 0; i < pix_count; i++, old_ptr += old_channel, tmp += channel)
            convert(tmp, old_ptr, channel, old_channel, 0);
    }
    free(old_ptr);
}

// Forward declaration section
term_u8 *crop_texture(term_u8 * old, term_u8 channel, term_ivec2 old_size,
                 term_ivec2 new_size);
term_u8 *resize_texture(const term_u8 * old, term_u8 channel, term_ivec2 old_size,
                   term_ivec2 new_size);

void tdt_merge(const term_texture *texture_a,
                   const term_texture *texture_b,
                   const term_ivec2 placement_pos,
                   const enum tdt_merge_mode mode, const term_bool replace)
{
    if (!texture_a || !texture_b)
        return;
    if (placement_pos.x > texture_a->size.x ||
        placement_pos.y > texture_a->size.y)
        return;
    term_u8 cha = texture_a->channel, chb = texture_b->channel;
    term_ivec2 sa = texture_a->size, sb = texture_b->size;
    term_u8 *ta =
        &texture_a->data[(placement_pos.y * sa.x + placement_pos.x) * cha],
        *tb = texture_b->data, *old = 0;

    // Apply size thersehold
    term_i32 max_space_x = sa.x - placement_pos.x, max_space_y =
        sa.y - placement_pos.y;
    term_u8 b_freeable = sb.x > max_space_x || sb.y > max_space_y;
    if (b_freeable) {
        term_ivec2 new_size =
            ivec2_init(sb.x > max_space_x ? max_space_x : sb.x,
                       sb.y > max_space_y ? max_space_y : sb.y);
        if (mode == TEXTURE_MERGE_RESIZE)
            tb = resize_texture(tb, chb, sb, new_size);
        else
            tb = crop_texture(tb, chb, sb, new_size);
        old = tb;
        sb = new_size;
    }

    term_i32 space = (sa.x - sb.x) * cha;
    for (term_i32 row = 0; row < sb.y; row++, ta += space) {
        for (term_i32 col = 0; col < sb.x; col++, ta += cha, tb += chb) {
            term_u8 tmp[4] = { 0 }, tmp_1 = chb;
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

term_ivec2 calculate_new_size(const term_ivec2 old, const term_ivec2 size)
{
    if (!size.x)
        return ivec2_init((old.x * size.y) / old.y, size.y);
    if (!size.y)
        return ivec2_init(size.x, (old.y * size.x) / old.x);
    return size;
}

term_u8 *resize_texture(const term_u8 *old, term_u8 channel, term_ivec2 old_size,
                   term_ivec2 new_size)
{
    if (!new_size.x || !new_size.y)
        new_size = calculate_new_size(old_size, new_size);
    return ptexture_resize(old, channel, old_size, new_size);
}

void tdt_resize(term_texture *texture, const term_ivec2 size)
{
    if (!texture)
        return;
    term_u8 *tmp =
        resize_texture(texture->data, texture->channel, texture->size,
                       size);
    if (!tmp)
        return;
    free(texture->data);
    texture->data = tmp;
    texture->size = calculate_new_size(texture->size, size);
}

term_bool tdt_resize_internal(term_texture *texture,
                           const term_ivec2 new_size)
{
    if (!texture)
        return 0;
    term_u8 *tmp =
        (term_u8 *) realloc(texture->data,
                       calculate_pos(0, new_size.y, new_size.x,
                                     texture->channel));
    if (!tmp)
        return 1;
    texture->data = tmp;
    texture->size = new_size;
    return 0;
}

term_u8 *crop_texture(term_u8 *old, term_u8 channel, term_ivec2 old_size,
                 term_ivec2 new_size)
{
    term_u8 *raw = 0;
    if (!
        (raw =
         (term_u8 *) malloc(calculate_pos(0, new_size.y, new_size.x, channel))))
        return 0;
    term_u8 *ptr = old, *start = raw;
    term_u64 row_length = (term_u64)(new_size.x * channel), old_length =
        (term_u64)(old_size.x * channel);
    for (term_i32 row = 0; row < new_size.y;
         row++, raw += row_length, ptr += old_length)
        memcpy(raw, ptr, row_length);
    return start;
}

void tdt_crop(term_texture *texture, const term_ivec2 new_size)
{
    if (!texture || new_size.x >= texture->size.x
        || new_size.y >= texture->size.y)
        return;
    term_u8 *tmp =
        crop_texture(texture->data, texture->channel, texture->size,
                     new_size);
    if (!tmp)
        return;
    free(texture->data);
    texture->data = tmp;
    texture->size = new_size;
}

void tdt_draw_line(term_texture *texture, const term_ivec2 p1,
                       const term_ivec2 p2, const term_rgba color)
{
    ptexture_draw_line(texture->data, texture->size, texture->channel,
                       p1, p2, vec2_init(0.0f, 0.0f), color,
                       0);
}

void tdt_free(term_texture *texture)
{
    if (!texture)
        return;
    if (texture->freeable)
        free(texture->data);
    free(texture);
}

term_rgba pixel_blend(term_rgba a, term_rgba b)
{
    term_u8 ar[4] = EXPAND_RGBA(a), br[4] = EXPAND_RGBA(b);
    alpha_blend(ar, br, 4, 4);
    return rgba_init(ar[0], ar[1], ar[2], ar[3]);
}
