#include "term_texture.h"

#include <string.h>

// Output result directly into a
// Only bit depth 8 is supported (1 byte for each channel)
void transparent_blend(u8* a, u8* b, u8 ch_in, u8 ch_out)
{
 u8 color[4] = {0};
 memcpy(color, b, ch_in);
 if(ch_in < 3 && ch_out > 2)
 {
  if(ch_in == 2)
   color[4] = color[1];
  color[0] = color[1] = color[2] = color[0];
  ch_in += 2;
 }
 if(ch_in > 2 && ch_out < 3)
 {
  color[0] = color[0] * 0.2989 + color[1] * 0.5870 + color[2] * 0.1140; // Convert to grayscale
  if(ch_in == 4)
   color[1] = color[3];
  ch_in -= 2;
 }
 u8 t = ch_out < 3 ? 1 : ((ch_out < 5) ? 3 : 0); // Exclude alpha because it have different formula for it
 const u8
  in_transparency = ch_in == 4 || ch_in == 2,
  out_transparency = ch_out == 4 || ch_out == 2,
  transparency = in_transparency || out_transparency;
 const u8
  alpha = transparency ? (in_transparency ? color[ch_in - 1] : a[ch_out-1]) : 255,
  inverse = 255 - alpha;
 for(u8 i = 0; i < t; i++)
  a[i] = transparency ? (alpha * color[i] + inverse * a[i]) >> 8 : color[i];
 if(out_transparency)
  a[ch_out-1] = alpha + ((inverse + a[ch_out-1]) >> 8);
}

struct term_rgba pixel_blend(struct term_rgba a, struct term_rgba b)
{
 u8 au[4] = {a.r,a.g,a.b,a.a}, bu[4] = {b.r,b.g,b.b,b.a};
 transparent_blend(au, bu, 4, 4);
 return rgba_init(au[0],au[1],au[2],au[3]);
}

void texture_fill_rgb(u8** texture, struct term_vec2 size, u8* color)
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
 u8 c[4] = {color.r, color.g, color.b, color.a};
 for(u32 row = 0; row < out.size.y; row++)
  for(u32 col = 0; col < out.size.x; col++)
   transparent_blend(&(*out.out_texture)[(row*out.size.x+col)*out.channel], c, 4, out.channel);
}

void texture_merge(struct texout out, struct texin in, struct term_vec2 pos)
{
 u8 transparent = out.channel == 4 || in.channel == 4;
}
