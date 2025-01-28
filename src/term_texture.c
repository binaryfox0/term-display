#include "term_texture.h"

#include <string.h>

#define IN_RANGE(value, first, last) ((value) >= (first) && (value) <= (last))
#define IS_TRANSPARENT(channel) ((channel) == 2 || (channel) == 4)
//https://stackoverflow.com/questions/27016781/is-performance-of-less-greater-than-than-better-than-less-greater-than-or-equ
#define IS_GRAYSCALE(channel) IN_RANGE(channel, 1, 2)
#define IS_TRUECOLOR(channel) IN_RANGE(channel, 3, 4)

u8 convert_grayscale(const u8* color)
{
 return color[0] * 0.2989 + color[1] * 0.5870 + color[2] * 0.1140;
}

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
  color[0] = convert_grayscale(color); // Convert to grayscale
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

void texture_fill(struct texinfo out, struct term_rgba color)
{
 if(
  !out.texture ||
  !color.a || // Alpha channel is 0, no more processing
  !IN_RANGE(out.channel, 1, 4)
 ) return;
 if(!*out.texture) return;
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
      &(*out.texture)[(row*out.size.x+col)*out.channel], c,
      4, out.channel
     );
    }
  }
  return;
 }
 texture_fill_rgb(*out.texture, out.size, c);
}

void texture_merge_truecolor(u8* start, u32 rlen, struct texinfo in, struct term_vec2 pos)
{
 for(u32 row = 0; row < in.size.x; row++)
   for(u32 col = 0; col < in.size.y; col++)
   {
    memset(
     &start[(row*rlen+(pos.x+col))*3],
     (*in.texture)[(row*in.size.x+col)], 3
    );
   }
}

void texture_merge_grayscale(u8* start, u32 rlen, struct texinfo in, struct term_vec2 pos)
{
 for(u32 row = 0; row < in.size.x; row++)
  for(u32 col = 0; col < in.size.y; col++)
  {
   start[(row*rlen+(pos.x+col))*3] = convert_grayscale(
    &(*in.texture)[(row*in.size.x+col)*3]);
  }
}

void texture_merge(struct texinfo out, struct texinfo in, struct term_vec2 pos)
{
 if(!IN_RANGE(out.channel, 1, 4) || !IN_RANGE(in.channel, 1, 4) ||
  !out.texture || !in.texture) return;
 if(!*out.texture) return;

 u8 *start_row = &(*out.texture)[pos.y*out.size.x*out.channel];
 if(IS_TRANSPARENT(out.channel) || IS_TRANSPARENT(in.channel))
 {
  for(u32 row = 0; row < in.size.y; row++)
  {
   for(u32 col = 0; col < in.size.x; col++)
   {
    transparent_blend(
     &start_row[(row*out.size.x+(pos.x+col))*out.channel],
     &(*in.texture)[(row*in.size.x+col)*in.channel],
     in.channel,
     out.channel
    );
   }
  }
  return;
 }
 if(IS_TRUECOLOR(out.channel)&&IS_GRAYSCALE(in.channel)){texture_merge_truecolor(start_row,out.size.x,in,pos);return;}
 if(IS_GRAYSCALE(out.channel)&&IS_TRUECOLOR(in.channel)){texture_merge_grayscale(start_row,out.size.x,in,pos);return;}

 // Now out.channel should equal to in.channel
 for(u32 row = 0; row < in.size.y; row++)
 {
  memcpy(
   &start_row[(row*out.size.x+pos.x)*out.channel],
   &in.texture[(row*in.size.x)*out.channel],
   in.size.x*out.channel
  );
 }
}
