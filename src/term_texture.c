#include "term_texture.h"

#include <stdlib.h>
#include <string.h>

struct term_texture_s
{
 u8 *data;
 u8 channel;
 term_vec2 size;
 u8 freeable;
};

/* Helper utilities start */
#define IS_TRANSPARENT(channel) ((channel) == 2 || (channel) == 4)
#define IS_GRAYSCALE(channel) ((channel) == 1 || (channel) == 2)
#define IS_TRUECOLOR(channel) ((channel) == 3 || (channel) == 4)

#define fast_floor(x) ((u32)(x))
#define fast_ceil(x) ((u32)(x) + ((x) > (u32)(x)))
/* Inline function start */
// Assuming texture have 24bpp (RGB) or 8bpp (Grayscale)
static inline u64 calculate_size(term_vec2 size, u8 channel) {return size.x*size.y*channel; }
static inline u8 to_grayscale(const u8* c) {return (77 * c[0] + 150 * c[1] + 29 * c[2]) >> 8;}
static inline u64 convert_pos(u32 x, u32 y, u32 width, u8 ch) {return (y*width+x)*ch;}
// Singular line, y discarded from the formula
static inline float lerp(u8 c0, u8 c1, float t) {return c0 + t * (c1 - c0);}
static inline u8 bilerp(u8 c00, u8 c10, u8 c01, u8 c11, float xt, float yt){
return lerp(lerp(c00, c10, xt),lerp(c01, c11, xt), yt);
}
/* Inline function end */

// Convert b to have the same type as a
void convert(u8* b_out, const u8* b_in, u8 ch_a, u8* ch_b)
{
 u8 a_g = IS_GRAYSCALE(ch_a), b_g = IS_GRAYSCALE(*ch_b);
 if(a_g && !b_g)
 {
  b_out[0] = to_grayscale(b_in);
  b_out[1] = *ch_b - 3 ? b_out[3] : 255;
  *ch_b = ch_a;
  return;
 }
 else if(!a_g && b_g)
 {
  b_out[3] = *ch_b - 1 ? b_in[1] : 255;
  b_out[0] = b_out[1] = b_out[2] = b_in[0];
  *ch_b = ch_a;
  return;
 }
 for(u8 i = 0; i < *ch_b; i++)
  b_out[i] = b_in[i];;
}

u8 convert_ch(u8 ch_a, u8 ch_b)
{
 u8 a_g = IS_GRAYSCALE(ch_a), b_g = IS_GRAYSCALE(ch_b);
 if(!a_g && b_g) return ch_b - 2;
 if(a_g && !b_g) return ch_b + 2;
 return ch_b;
}

void alpha_blend(u8* a, u8* b, u8 ch_a, u8 ch_b)
{
 u8 out_a = IS_TRANSPARENT(ch_a);
 u8 a_i = ch_a - 1;
 u8 a_a = out_a ? a[ch_a - 1] : 255;
 u16 a_b = IS_TRANSPARENT(ch_b) ? b[ch_b - 1] : 255, iva_b = 255 - a_b;
 if(ch_a < 5)
  a[0] = (a_b * b[0] + iva_b * a[0]) >> 8;
 if(ch_a > 2)
  a[1] = (a_b * b[1] + iva_b * a[1]) >> 8;
  a[2] = (a_b * b[2] + iva_b * a[2]) >> 8;
 if(out_a)
  a[a_i] = !iva_b ? 255 : a_b + ((iva_b + a[a_i]) >> 8);
}
/* Helper utilities end   */

term_texture* texture_create(
 u8* texture,
 const u8 channel,
 const term_vec2 size,
 const u8 freeable,
 const u8 copy
)
{
 if(
  OUT_RANGE(channel, 1, 4) ||
  !size.x || !size.y
 ) return 0;
 term_texture* out = 0;
 if(!(out = (term_texture*)malloc(sizeof(term_texture))))
  return 0;
 u64 alloc_size = calculate_size(size, channel);
 
 if (!texture || copy)
 {
  out->data = (u8*)malloc(alloc_size);
  if (!out->data)
  {
   free(out);
   return 0;
  }
  out->freeable = 1;
  if (!texture)
   memset(out->data, 0, alloc_size);
  else
  {
   memcpy(out->data, texture, alloc_size);
   if(freeable) free(texture);
  }
 }
 else
 {
  out->data = texture;
  out->freeable = freeable;
 }

 out->size = size;
 out->channel = channel;
 return out;
}

term_texture* texture_copy(term_texture* texture)
{
 term_texture* out = 0;
 if(!(out = (term_texture*)malloc(sizeof(term_texture))))
  return 0;
 u64 size = texture->size.x * texture->size.y * texture->channel;
 if(!(out->data = (u8*)malloc(size)))
 {
  free(out);
  return 0;
 }
 memcpy(out->data, texture->data, size);
 out->freeable = 1;
 out->size = texture->size;
 out->channel = texture->channel;
 return out;
}

u8* texture_get_location(const term_vec2 pos, const term_texture* texture)
{
 if(!texture || pos.x >= texture->size.x || pos.y >= texture->size.y)
  return 0;
 return &(texture->data[(pos.y * texture->size.x + pos.x) * texture->channel]);
}

static inline term_vec2 texture_get_size(const term_texture* texture)
{
 if(!texture)
  return vec2_init(0,0);
 return texture->size;
}

// Only support for the same color type as texture
void exponentially_fill(u8* data, u64 size, u8* c, u8 ch)
{
 memcpy(data, c, ch);
 u64 filled = ch;
 while(filled * 2 < size)
 {
  memcpy(&data[filled], data, filled);
  filled *= 2;
 }
 memcpy(&data[filled], data, size - filled);
}

void texture_fill(const term_texture* texture, const term_rgba color)
{
 if(!texture || !color.a)
  return;
 u8 c[4] = EXPAND_RGBA(color), tmp = 4;
 term_vec2 size = texture->size;
 u8* data = texture->data;
 u8 ch = texture->channel;

 convert(c, c, ch, &tmp);
 if(IS_TRANSPARENT(ch) || color.a != 255)
 {
  for(u32 row = 0; row < size.y; row++)
  {
   for(u32 col = 0; col < size.x; col++, data += ch)
    alpha_blend(data, c, ch, 4);
  }
  return;
 }
 exponentially_fill(data, size.x*size.y*ch, c, ch);
}


// Forward declaration section
u8* crop_texture(u8* old, u8 channel, term_vec2 old_size, term_vec2 new_size);
u8* resize_texture(const u8* old, u8 channel, term_vec2 old_size, term_vec2 new_size);

void texture_merge(
 const term_texture* texture_a,
 const term_texture* texture_b,
 const term_vec2 placement_pos,
 const enum texture_merge_mode mode,
 const u8 replace
)
{
 if(!texture_a || !texture_b) return;
 if(
   placement_pos.x > texture_a->size.x - texture_b->size.x ||
   placement_pos.y > texture_a->size.y - texture_b->size.y) return;
 u8 cha = texture_a->channel, chb = texture_b->channel;
 term_vec2 sa = texture_a->size, sb = texture_b->size;
 u8 *ta = &texture_a->data[(placement_pos.y*sa.x+placement_pos.x)*cha], *tb = texture_b->data, *old = 0;

 // Apply size thersehold
 u64 max_space_x = sa.x - placement_pos.x, max_space_y = sa.y - placement_pos.y;
 u8 b_freeable = sb.x > max_space_x || sb.y > max_space_y;
 if(b_freeable)
 {
  term_vec2 new_size = vec2_init(sb.x > max_space_x ? max_space_x : sb.x, sb.y > max_space_y ? max_space_y : sb.y);
  if(mode == TEXTURE_MERGE_RESIZE)
   tb = resize_texture(tb, chb, sb, new_size);
  else
   tb = crop_texture(tb, chb, sb, new_size);
  old = tb;
  sb = new_size;
 }
 
 u32 space = (sa.x - sb.x) * cha;
 for(u32 row = 0; row < sb.y; row++, ta += space)
 {
  for(u32 col = 0; col < sb.x; col++, ta += cha, tb += chb)
  {
   u8 tmp[4] = {0}, tmp_1 = chb;
   convert(tmp, tb, cha, &tmp_1);
   replace ? memcpy(ta, tmp, cha) : alpha_blend(ta, tmp, cha, tmp_1);
  }
 }
 if(b_freeable) free(old);
}

term_vec2 calculate_new_size(const term_vec2 old, const term_vec2 size)
{
 if(!size.x) return vec2_init((old.x * size.y) / old.y ,size.y);
 if(!size.y) return vec2_init(size.x, (old.y * size.x) / old.x);
 return size;
}

u8* resize_texture(const u8* old, u8 channel, term_vec2 old_size, term_vec2 new_size)
{
 if(!new_size.x || !new_size.y) new_size = calculate_new_size(old_size, new_size);
 float
  x_ratio = (float)(old_size.x - 1) / (new_size.x - 1),
  y_ratio = (float)(old_size.y - 1) / (new_size.y - 1);
 u8 *raw = (u8*)malloc(calculate_size(new_size, channel)), *start = raw;
 if(!raw) return 0;

 for (u32 row = 0; row < new_size.y; row++)
 {
  float tmp = row * y_ratio;
  u32 iyf = fast_floor(tmp),
      iyc = fast_ceil(tmp);
  float ty = tmp - iyf;
  for (u32 col = 0; col < new_size.x; col++)
  {
   tmp = col * x_ratio;
   u32 ixf = fast_floor(tmp),
       ixc = fast_ceil(tmp);
   float tx = tmp - ixf;

   u32 i00 = convert_pos(ixf, iyf, old_size.x, channel),
       i10 = convert_pos(ixc, iyf, old_size.x, channel),
       i01 = convert_pos(ixf, iyc, old_size.x, channel),
       i11 = convert_pos(ixc, iyc, old_size.x, channel);

   for(u8 c = 0; c < channel; c++, raw++)
    raw[0] = bilerp(old[i00+c], old[i10+c], old[i01+c], old[i11+c], tx, ty);
  }
 }
 return start;
}

//https://gist.github.com/folkertdev/6b930c7a7856e36dcad0a72a03e66716
void texture_resize(term_texture* texture, const term_vec2 size)
{
 if(!texture) return;
 u8* tmp = resize_texture(texture->data, texture->channel, texture->size, size);
 if(!tmp) return;
 free(texture->data);
 texture->data = tmp;
 texture->size = size;
}

u8 texture_resize_internal(term_texture* texture, const term_vec2 new_size)
{
 if(!texture) return 0;
 u8* tmp = (u8*)realloc(texture->data, calculate_size(new_size, texture->channel));
 if(!tmp) return 1;
 texture->data = tmp;
 texture->size = new_size;
 return 0;
}

u8* crop_texture(u8* old, u8 channel, term_vec2 old_size, term_vec2 new_size)
{
 u8* raw = 0;
 if(!(raw = (u8*)malloc(calculate_size(new_size, channel))))
  return 0;
 u8* ptr = old, *start = raw;
 u64 row_length = new_size.x * channel, old_length = old_size.x * channel;
 for(u32 row = 0; row < new_size.y; row++, raw += row_length, ptr += old_length)
  memcpy(raw, ptr, row_length);
 return start;
}

void texture_crop(term_texture *texture, const term_vec2 new_size)
{
 if(!texture || new_size.x >= texture->size.x || new_size.y >= texture->size.y) return;
 u8* tmp = crop_texture(texture->data, texture->channel, texture->size, new_size);
 if(!tmp) return;
 free(texture->data);
 texture->data = tmp;
 texture->size = new_size;
}

void texture_free(term_texture* texture)
{
 if(!texture)
  return;
 if(texture->freeable)
  free(texture->data);
 free(texture);
}

term_rgba pixel_blend(term_rgba a, term_rgba b)
{
 u8 ar[4] = EXPAND_RGBA(a), br[4] = EXPAND_RGBA(b);
 alpha_blend(ar, br, 4, 4);
 return rgba_init(ar[0], ar[1], ar[2], ar[3]);
}
