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

struct term_texture_s;
typedef struct term_texture_s term_texture;

/* Constructor start */
term_texture *tdt_create(td_u8 * texture,
                             const td_u8 channel,
                             const term_ivec2 size,
                             const td_u8 freeable, const td_u8 copy);

term_texture *tdt_copy(term_texture * texture);
/* Constructor end */

td_u8 *tdt_get_location(const term_ivec2 pos,
                         const term_texture * texture);

term_ivec2 tdt_get_size(const term_texture * texture);

void tdt_fill(const term_texture * texture, const term_rgba color);

void tdt_set_channel(term_texture* texture, td_u8 channel);

enum tdt_merge_mode {
    TEXTURE_MERGE_CROP = 0,
    TEXTURE_MERGE_RESIZE
};

// Texture b will place over texture a with proper blending
void tdt_merge(const term_texture * texture_a,
                   const term_texture * texture_b,
                   const term_ivec2 placment_pos,
                   const enum tdt_merge_mode mode, const term_bool replace);

// Resizing texture with bilinear interpolation
void tdt_resize(term_texture * texture, const term_ivec2 new_size);
// Resizing only the raw texture storage
term_bool tdt_resize_internal(term_texture * texture,
                           const term_ivec2 new_size);
void tdt_crop(term_texture * texture, const term_ivec2 new_size);

void tdt_draw_line(term_texture * texture, const term_ivec2 p1,
                       const term_ivec2 p2, const term_rgba color);

void tdt_free(term_texture * texture);

// Additional functions
term_rgba pixel_blend(term_rgba a, term_rgba b);

#endif
