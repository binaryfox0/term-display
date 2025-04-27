#ifndef TD_TEXTURE_H
#define TD_TEXTURE_H

#include "td_def.h"

struct term_texture_s;
typedef struct term_texture_s term_texture;

/* Constructor start */
term_texture *tdt_create(term_u8 * texture,
                             const term_u8 channel,
                             const term_ivec2 size,
                             const term_u8 freeable, const term_u8 copy);

term_texture *tdt_copy(term_texture * texture);
/* Constructor end */

term_u8 *tdt_get_location(const term_ivec2 pos,
                         const term_texture * texture);

term_ivec2 tdt_get_size(const term_texture * texture);

void tdt_fill(const term_texture * texture, const term_rgba color);

void tdt_set_channel(term_texture* texture, term_u8 channel);

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
