#ifndef TERMINAL_TEXTURE_H
#define TERMINAL_TEXTURE_H

#include "term_def.h"

struct term_texture_s;
typedef struct term_texture_s term_texture;

/* Constructor start */
term_texture* texture_create(
 u8* texture,
 const u8 channel,
 const term_vec2 size,
 const u8 freeable,
 const u8 copy
);

term_texture* texture_copy(term_texture* texture);
/* Constructor end */

u8* texture_get_location(
 const term_vec2 pos,
 const term_texture* texture
);

static inline term_vec2 texture_get_size(const term_texture* texture);

/* Texture editing function start */
void texture_fill(const term_texture* texture, const term_rgba color);

enum texture_merge_mode
{
 TEXTURE_MERGE_CROP = 0,
 TEXTURE_MERGE_RESIZE
};

// Texture b will place over texture a with proper blending
void texture_merge(
 const term_texture* texture_a,
 const term_texture* texture_b,
 const term_vec2 placment_pos,
 const enum texture_merge_mode mode,
 const u8 replace
);

// Resizing texture with bilinear interpolation
void texture_resize(term_texture* texture, const term_vec2 new_size);
// Resizing only the raw texture storage
u8 texture_resize_internal(term_texture* texture, const term_vec2 new_size);
void texture_crop(term_texture* texture, const term_vec2 new_size);
/* Texture editing function end */

void texture_free(term_texture* texture);

// Additional functions
term_rgba pixel_blend(term_rgba a, term_rgba b);

#endif
