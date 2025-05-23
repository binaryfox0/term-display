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

#ifndef TD_TEXTURE_H
#define TD_TEXTURE_H

#include "td_def.h"

struct td_texture_s;
typedef struct td_texture_s td_texture;

/* Constructor start */
td_texture *tdt_create(td_u8 * texture,
                             const td_u8 channel,
                             const td_ivec2 size,
                             const td_u8 freeable, const td_u8 copy);

td_texture *tdt_copy(td_texture * texture);
/* Constructor end */

td_u8 *tdt_get_location(const td_ivec2 pos,
                         const td_texture * texture);

td_ivec2 tdt_get_size(const td_texture * texture);

void tdt_fill(const td_texture * texture, const td_rgba color);

void tdt_set_channel(td_texture* texture, td_u8 channel);

enum tdt_merge_mode {
    TEXTURE_MERGE_CROP = 0,
    TEXTURE_MERGE_RESIZE
};

// Texture b will place over texture a with proper blending
void tdt_merge(const td_texture * texture_a,
                   const td_texture * texture_b,
                   const td_ivec2 placment_pos,
                   const enum tdt_merge_mode mode, const td_bool replace);

// Resizing texture with bilinear interpolation
void tdt_resize(td_texture * texture, const td_ivec2 new_size);
// Resizing only the raw texture storage
td_bool tdt_resize_internal(td_texture * texture,
                           const td_ivec2 new_size);
void tdt_crop(td_texture * texture, const td_ivec2 new_size);

void tdt_draw_line(td_texture * texture, const td_ivec2 p1,
                       const td_ivec2 p2, const td_rgba color);

void tdt_free(td_texture * texture);

// Additional functions
td_rgba pixel_blend(td_rgba a, td_rgba b);

#endif
