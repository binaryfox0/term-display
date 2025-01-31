#ifndef TERMINAL_TEXTURE_H
#define TERMINAL_TEXTURE_H

#include "term_def.h"

struct texinfo
{
 u8** texture;
 u8 channel;
 struct term_vec2 size;
};

static inline struct texinfo texinfo_init(u8** texture, u8 channel, struct term_vec2 size)
{
 return (struct texinfo) { .texture = texture, .channel = channel, .size = size };
}

/* Developer notes */
// We recommend you use raw pointer for texture, struct may have pad
// disabling it can result in bad performance, which
// program want to prevent it as much as possible
struct term_rgba pixel_blend(struct term_rgba a, struct term_rgba b);
void texture_fill(struct texinfo out, struct term_rgba color);
void texture_merge(struct texinfo out, struct texinfo in, struct term_vec2 pos);
#endif

