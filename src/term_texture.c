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

#include "term_texture.h"

#include "term_priv.h"
#include "term_texture_priv.h"

#include <stdlib.h>
#include <string.h>

struct term_texture_s {
    u8 *data;
    u8 channel;
    term_ivec2 size;
    u8 freeable;
};

/* Helper utilities start */

#define fast_floor(x) ((u32)(x))
#define fast_ceil(x) ((u32)(x) + ((x) > (u32)(x)))
/* Inline function start */
// Assuming texture have 24bpp (RGB) or 8bpp (Grayscale)
// Singular line, y discarded from the formula
static inline u8 bilerp(u8 c00, u8 c10, u8 c01, u8 c11, float xt, float yt)
{
    return (u8) lerp(lerp(c00, c10, xt), lerp(c01, c11, xt), yt);
}

/* Inline function end */



u8 convert_ch(u8 ch_a, u8 ch_b)
{
    u8 a_g = IS_GRAYSCALE(ch_a), b_g = IS_GRAYSCALE(ch_b);
    if (!a_g && b_g)
        return ch_b - 2;
    if (a_g && !b_g)
        return ch_b + 2;
    return ch_b;
}

/* Helper utilities end   */

term_texture *texture_create(u8 *texture,
                             const u8 channel,
                             const term_ivec2 size,
                             const u8 freeable, const u8 copy)
{
    if (OUT_RANGE(channel, 1, 4) || !size.x || !size.y)
        return 0;
    term_texture *out = 0;
    if (!(out = (term_texture *) malloc(sizeof(term_texture))))
        return 0;
    u64 alloc_size = calculate_pos(0, size.y, size.x, channel);

    if (!texture || copy) {
        out->data = (u8 *) malloc(alloc_size);
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

term_texture *texture_copy(term_texture *texture)
{
    term_texture *out = 0;
    if (!(out = (term_texture *) malloc(sizeof(term_texture))))
        return 0;
    u64 size = texture->size.x * texture->size.y * texture->channel;
    if (!(out->data = (u8 *) malloc(size))) {
        free(out);
        return 0;
    }
    memcpy(out->data, texture->data, size);
    out->freeable = 1;
    out->size = texture->size;
    out->channel = texture->channel;
    return out;
}

u8 *texture_get_location(const term_ivec2 pos, const term_texture *texture)
{
    if (!texture || pos.x >= texture->size.x || pos.y >= texture->size.y)
        return 0;
    return &(texture->
             data[(pos.y * texture->size.x + pos.x) * texture->channel]);
}

term_ivec2 texture_get_size(const term_texture *texture)
{
    if (!texture)
        return ivec2_init(0, 0);
    return texture->size;
}

// Only support for the same color type as texture
void exponentially_fill(u8 *data, u64 size, u8 *c, u8 ch)
{
    memcpy(data, c, ch);
    u64 filled = ch;
    while (filled * 2 < size) {
        memcpy(&data[filled], data, filled);
        filled *= 2;
    }
    memcpy(&data[filled], data, size - filled);
}

void texture_fill(const term_texture *texture, const term_rgba color)
{
    if (!texture || !color.a)
        return;
    u8 c[4] = EXPAND_RGBA(color), tmp = 4;
    term_ivec2 size = texture->size;
    u8 *data = texture->data;
    u8 ch = texture->channel;

    convert(c, c, ch, 4, &tmp);
    if (IS_TRANSPARENT(ch) || color.a != 255) {
        for (u32 row = 0; row < size.y; row++) {
            for (u32 col = 0; col < size.x; col++, data += ch)
                alpha_blend(data, c, ch, 4);
        }
        return;
    }
    exponentially_fill(data, size.x * size.y * ch, c, ch);
}


// Forward declaration section
u8 *crop_texture(u8 * old, u8 channel, term_ivec2 old_size,
                 term_ivec2 new_size);
u8 *resize_texture(const u8 * old, u8 channel, term_ivec2 old_size,
                   term_ivec2 new_size);

void texture_merge(const term_texture *texture_a,
                   const term_texture *texture_b,
                   const term_ivec2 placement_pos,
                   const enum texture_merge_mode mode, const u8 replace)
{
    if (!texture_a || !texture_b)
        return;
    if (placement_pos.x > texture_a->size.x ||
        placement_pos.y > texture_a->size.y)
        return;
    u8 cha = texture_a->channel, chb = texture_b->channel;
    term_ivec2 sa = texture_a->size, sb = texture_b->size;
    u8 *ta =
        &texture_a->data[(placement_pos.y * sa.x + placement_pos.x) * cha],
        *tb = texture_b->data, *old = 0;

    // Apply size thersehold
    u64 max_space_x = sa.x - placement_pos.x, max_space_y =
        sa.y - placement_pos.y;
    u8 b_freeable = sb.x > max_space_x || sb.y > max_space_y;
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

    u32 space = (sa.x - sb.x) * cha;
    for (u32 row = 0; row < sb.y; row++, ta += space) {
        for (u32 col = 0; col < sb.x; col++, ta += cha, tb += chb) {
            u8 tmp[4] = { 0 }, tmp_1 = chb;
            convert(tmp, tb, cha, chb, &tmp_1);
            replace ? memcpy(ta, tmp, cha) : alpha_blend(ta, tmp, cha,
                                                         tmp_1);
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

u8 *resize_texture(const u8 *old, u8 channel, term_ivec2 old_size,
                   term_ivec2 new_size)
{
    if (!new_size.x || !new_size.y)
        new_size = calculate_new_size(old_size, new_size);
    float
        x_ratio = (float) (old_size.x - 1) / (new_size.x - 1),
        y_ratio = (float) (old_size.y - 1) / (new_size.y - 1);
    u8 *raw =
        (u8 *) malloc(calculate_pos(0, new_size.y, new_size.x, channel)),
        *start = raw;
    if (!raw)
        return 0;

    for (u32 row = 0; row < new_size.y; row++) {
        float tmp = row * y_ratio;
        u32 iyf = fast_floor(tmp), iyc = fast_ceil(tmp);
        float ty = tmp - iyf;
        for (u32 col = 0; col < new_size.x; col++) {
            tmp = col * x_ratio;
            u32 ixf = fast_floor(tmp), ixc = fast_ceil(tmp);
            float tx = tmp - ixf;

            u32 i00 = calculate_pos(ixf, iyf, old_size.x, channel),
                i10 = calculate_pos(ixc, iyf, old_size.x, channel),
                i01 = calculate_pos(ixf, iyc, old_size.x, channel),
                i11 = calculate_pos(ixc, iyc, old_size.x, channel);

            for (u8 c = 0; c < channel; c++, raw++)
                raw[0] =
                    bilerp(old[i00 + c], old[i10 + c], old[i01 + c],
                           old[i11 + c], tx, ty);
        }
    }
    return start;
}

//https://gist.github.com/folkertdev/6b930c7a7856e36dcad0a72a03e66716
void texture_resize(term_texture *texture, const term_ivec2 size)
{
    if (!texture)
        return;
    u8 *tmp =
        resize_texture(texture->data, texture->channel, texture->size,
                       size);
    if (!tmp)
        return;
    free(texture->data);
    texture->data = tmp;
    texture->size = calculate_new_size(texture->size, size);
}

u8 texture_resize_internal(term_texture *texture,
                           const term_ivec2 new_size)
{
    if (!texture)
        return 0;
    u8 *tmp =
        (u8 *) realloc(texture->data,
                       calculate_pos(0, new_size.y, new_size.x,
                                     texture->channel));
    if (!tmp)
        return 1;
    texture->data = tmp;
    texture->size = new_size;
    return 0;
}

u8 *crop_texture(u8 *old, u8 channel, term_ivec2 old_size,
                 term_ivec2 new_size)
{
    u8 *raw = 0;
    if (!
        (raw =
         (u8 *) malloc(calculate_pos(0, new_size.y, new_size.x, channel))))
        return 0;
    u8 *ptr = old, *start = raw;
    u64 row_length = new_size.x * channel, old_length =
        old_size.x * channel;
    for (u32 row = 0; row < new_size.y;
         row++, raw += row_length, ptr += old_length)
        memcpy(raw, ptr, row_length);
    return start;
}

void texture_crop(term_texture *texture, const term_ivec2 new_size)
{
    if (!texture || new_size.x >= texture->size.x
        || new_size.y >= texture->size.y)
        return;
    u8 *tmp =
        crop_texture(texture->data, texture->channel, texture->size,
                     new_size);
    if (!tmp)
        return;
    free(texture->data);
    texture->data = tmp;
    texture->size = new_size;
}

void texture_draw_line(term_texture *texture, const term_ivec2 p1,
                       const term_ivec2 p2, const term_rgba color)
{
    ptexture_draw_line(texture->data, texture->size, texture->channel,
                       p1, p2, vec2_init(0.0f, 0.0f), color,
                       0);
}

void texture_free(term_texture *texture)
{
    if (!texture)
        return;
    if (texture->freeable)
        free(texture->data);
    free(texture);
}

term_rgba pixel_blend(term_rgba a, term_rgba b)
{
    u8 ar[4] = EXPAND_RGBA(a), br[4] = EXPAND_RGBA(b);
    alpha_blend(ar, br, 4, 4);
    return rgba_init(ar[0], ar[1], ar[2], ar[3]);
}
