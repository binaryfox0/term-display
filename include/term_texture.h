#ifndef TERMINAL_TEXTURE_H
#define TERMINAL_TEXTURE_H

#include "term_def.h"

struct texout
{
 u8** out_texture;
 u8 channel;
 struct term_vec2 size;
};

struct texin
{
 const u8* in_texture;
 const u8 channel;
 const struct term_vec2 size;
};

static inline struct texout texout_init(u8** texture, u8 channel, struct term_vec2 size)
{
 return (struct texout) { .out_texture = texture, .channel = channel, .size = size };
}

static inline struct texin texin_init(const u8* texture, const u8 channel, const struct term_vec2 size)
{
 return (struct texin) { .in_texture = texture, .channel = channel, .size = size };
}

// Texture as 1D-array continious case only
struct term_rgba pixel_blend(struct term_rgba a, struct term_rgba b);
void texture_fill(struct texout out, struct term_rgba color);
void texture_merge(struct texout out, struct texin in, struct term_vec2 pos);
#endif

