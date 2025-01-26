#include "term_texture.h"

#include <string.h>

#define IN_RANGE(value, first, last) ((value) >= (first) && (value) <= (last))
#define IS_TRANSPARENT(channel) ((channel) == 2 || (channel) == 4)

// Output result directly into a
// Only bit depth 8 is supported (1 byte for each channel)
void transparent_blend(u8* a, const u8* b, u8 ch_in, u8 ch_out)
{
 u8 color[4] = {0};
 if(IN_RANGE(ch_in, 1, 2) && IN_RANGE(ch_out, 3, 4))
 {
  if(ch_in == 2)
   color[3] = color[1];
  color[0] = color[1] = color[2] = color[0];
  ch_in += 2;
 }
 else if(IN_RANGE(ch_in, 3, 4) && IN_RANGE(ch_out, 1, 2))
 {
  color[0] = color[0] * 0.2989 + color[1] * 0.5870 + color[2] * 0.1140; // Convert to grayscale
  ch_in -= 2;
  if(ch_in == 2)
   color[1] = color[3];
 }
 else if (IN_RANGE(ch_in, 1, 4)) { memcpy(color, b, ch_in); }
 else return;
 u8 t = ch_out < 3 ? 1 : ((ch_out < 5) ? 3 : 0); // Exclude alpha because it have different formula for it
 const u8
  in_transparency = IS_TRANSPARENT(ch_in),
  out_transparency = IS_TRANSPARENT(ch_out),
  transparency = in_transparency || out_transparency;
 const u8
  in_transparency_i = ch_in - 1,
  out_transparency_i = ch_out - 1;
 const u8
  alpha = transparency ? (in_transparency ? color[in_transparency_i] : a[out_transparency_i]) : 255,
  inverse = 255 - alpha;

 for(u8 i = 0; i < t; i++)
  a[i] = transparency ? (alpha * color[i] + inverse * a[i]) >> 8 : color[i];
 if(out_transparency)
  a[out_transparency_i] = alpha + ((inverse + a[out_transparency_i]) >> 8);
}

struct term_rgba pixel_blend(struct term_rgba a, struct term_rgba b)
{
 u8 au[4] = {a.r,a.g,a.b,a.a}, bu[4] = {b.r,b.g,b.b,b.a};
 transparent_blend(au, bu, 4, 4);
 return rgba_init(au[0],au[1],au[2],au[3]);
}

void texture_fill_rgb(u8* texture, struct term_vec2 size, u8* color)
{
 if(!size.x || !size.y)
  return;
 u32 bytes_count = size.x * size.y * 3;
 memcpy(texture, color, 3);
 // Exponentially filling display
 u32 filled = 3;
 while(filled < bytes_count - filled)
 {
  memcpy(&texture[filled], texture, filled);
  filled *= 2;
 }
 // Filling the remaining
 memcpy(&texture[filled], texture, (bytes_count - filled));
}

void texture_fill(struct texout out, struct term_rgba color)
{
 if(
  !out.out_texture ||
  !color.a || // Alpha channel is 0, no more processing
  !IN_RANGE(out.channel, 1, 4)
 ) return;
 u8 c[4] = {color.r, color.g, color.b, color.a};
 if(
  c[3] != 255 ||
  IS_TRANSPARENT(out.channel)
 )
 {
  for(u32 row = 0; row < out.size.y; row++)
  {
    for(u32 col = 0; col < out.size.x; col++)
    {
     transparent_blend(
      &out.out_texture[(row*out.size.x+col)*out.channel], c,
      4, out.channel
     );
    }
  }
  return;
 }
 texture_fill_rgb(out.out_texture, out.size, c);
}

void texture_merge(struct texout out, struct texin in, struct term_vec2 pos)
{
 u8 *start_row = &out.out_texture[pos.y*out.size.x*out.channel];
 for(u32 row = 0; row < in.size.y; row++)
 {
  for(u32 col = 0; col < in.size.x; col++)
  {
   transparent_blend(
    &start_row[(row*out.size.x+(pos.x+col))*out.channel],
    &in.in_texture[(row*in.size.x+col)*in.channel],
    in.channel,
    out.channel
   );
  }
 }
}
