#include "term_texture.h"

#include <string.h>

#define IN_RANGE(value, first, last) ((value) >= (first) && (value) <= (last))
#define IS_TRANSPARENT(channel) ((channel) == 2 || (channel) == 4)

// Output directly into a
// This is b over a
void transparent_blending(u8* a, u8* b, u8 ch_in, u8 ch_out)
{
 u8 color[4] = {0};
 if(IN_RANGE(ch_in, 3, 4) && IN_RANGE(ch_out, 1, 2))
 {
  color[0] = b[0] * 0.2989 + b[1] * 0.5870 + b[2] * 0.1140;
  ch_in -= 2;
  if(ch_in == 2)
   color[1] = b[3];
 } else if(IN_RANGE(ch_in, 1, 2) && IN_RANGE(ch_out, 3, 4))
 {
  color[0] = color[1] = color[2] = b[0];
  if(ch_in == 2)
   color[3] = b[1];
  ch_in += 2;
 } else if(IN_RANGE(ch_in, 0, 4))
 {
  for(u8 i = 0; i < ch_in; i++)
   color[i] = b[i];
 } else return;
 u8 t = ch_in < 3 ? 1 : (ch_in < 5 ? 3 : 0);
 u8 in_o = IS_TRANSPARENT(ch_in),
   out_o = IS_TRANSPARENT(ch_out),
   o = in_o || out_o;
 u8 in_o_p = ch_in - 1, out_o_p = ch_out - 1;
 u16 alpha = o ? (in_o ? color[in_o_p] : a[out_o_p]) : 255,
  inverse = 255 - alpha; // Use u16 to prevent integer wrapping
 for(u8 i = 0; i < t; i++)
  a[i] = in_o ? (alpha * color[i] + inverse * a[i]) >> 8 : color[i];
 if(out_o)
  a[out_o_p] = in_o ? alpha + ((a[out_o_p] * inverse) >> 8) : 255;
}

struct term_rgba pixel_blend(struct term_rgba a, struct term_rgba b)
{
 u8 au[4] = {a.r, a.g, a.b, a.a}, bu[4] = {b.r,b.g,b.b,b.a};
 transparent_blending(au, bu, 4, 4);
 return rgba_init(au[0], au[1], au[2], au[3]);
}

void texture_fill_rgb(u8** texture, struct term_vec2 size, struct term_rgb color)
{
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
 if(
  !out.out_texture ||
  !color.a ||
  !out.size.x || out.size.y
 ) return;
 if(IS_TRANSPARENT(out.channel) || color.a != 255)
 {
  u8 c[4] = {color.r, color.g, color.b, color.a};
  for(u32 row = 0; row < out.size.y; row++)
  {
   for(u32 col = 0; col < out.size.x; col++)
    transparent_blending(&(*out.out_texture)[(row*out.size.x+col)*out.channel], c, 4, out.channel);
  }
  return;
 }
 texture_fill_rgb(out.out_texture, out.size, to_rgb(color));
}

void texture_merge(struct texout out, struct texin in, struct term_vec2 pos)
{
 u8 transparent = out.channel == 4 || in.channel == 4;
}
