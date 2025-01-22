#include "term_texture.h"

#include <string.h>

struct term_rgba pixel_blend(struct term_rgba a, struct term_rgba b)
{
 u16 inverse_alpha = 255 - b.a; // Prevent integer wrapping
 rgba_init(
  (b.a * b.r + inverse_alpha * a.r) >> 8,
  (b.a * b.g + inverse_alpha * a.g) >> 8,
  (b.a * b.b + inverse_alpha * a.b) >> 8,
  b.a + ((inverse_alpha * a.a) >> 8)
 );
}

struct term_rgb transparent_blend_normal(struct term_rgb a, struct term_rgba b)
{
 u16 inverse_alpha = 255 - b.a; // Prevent integer wrapping
 return rgb_init(
  (b.a * b.r + inverse_alpha * a.r) >> 8,
  (b.a * b.g + inverse_alpha * a.g) >> 8,
  (b.a * b.b + inverse_alpha * a.b) >> 8
 );
}

void texture_fill_rgb(u8** texture, struct term_vec2 size, struct term_rgb color)
{
 if(!size.x || !size.y)
  return;
 u32 bytes_count = size.x * size.y * 3;
 memcpy(*texture, &color, 3);
 // Exponentially filling display
 u32 filled = 3;
 while(filled < bytes_count - filled)
 {
  memcpy(&(*texture)[filled], *texture, filled);
  filled *= 2;
 }
 // Filling the remaining
 memcpy(&(*texture)[filled], *texture, (bytes_count - filled));
}

void texture_fill(struct texout out, struct term_rgba color)
{
 if(!out.out_texture) return;
 u8 transparent = out.channel == 4 || color.a != 255;
 // Channels smaller than rgb don,t support now
 if(transparent)
 {
  if(out.channel == 3)
  {
   for(u32 row = 0; row < out.size.y; row++)
    for(u32 col = 0; col < out.size.x; col++)
    {
     struct term_rgb* ptr = (struct term_rgb*)&((*out.out_texture)[(row * out.size.x + col) * 3]);
     *ptr = transparent_blend_normal(*ptr, color);
    }
   return;
  }
  if(out.channel == 4)
   for(u32 row = 0; row < out.size.y; row++)
    for(u32 col = 0; col < out.size.x; col++)
    {
     struct term_rgba* ptr = (struct term_rgba*)&((*out.out_texture)[(row * out.size.x + col) * 4]);
     *ptr = pixel_blend(*ptr, color);
    }
  return;
 }
 texture_fill_rgb(out.out_texture, out.size, to_rgb(color));
}

void texture_merge(struct texout out, struct texin in, struct term_vec2 pos)
{
 u8 transparent = out.channel == 4 || in.channel == 4;
}
