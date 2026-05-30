#ifndef TD_TEXTURE_PRIV_H
#define TD_TEXTURE_PRIV_H

#include <td_def.h>
#include <td_texture.h>

#define TDP_HAS_ALPHA(type) \
    ((type) == TD_TEXTURE_GRAY_ALPHA || (type) == TD_TEXTURE_RGB_ALPHA)
#define TDP_IS_GRAY(type) \
    ((type) == TD_TEXTURE_GRAY || (type) == TD_TEXTURE_GRAY_ALPHA)
#define TDP_IS_RGB(type) \
    ((type) == TD_TEXTURE_RGB || (type) == TD_TEXTURE_RGB_ALPHA)

typedef struct td_texture 
{
    td_u8 *data;
    td_texture_type_t type;
    td_ivec2 size;
    td_bool freeable;
    td_bool lib_owned; 
} td_texture_t;

void tdp_blend(
        td_u8 *a, 
        const td_u8 *b, 
        const td_i32 ch_a, 
        const td_i32 ch_b);

void tdp_convert_color(
        td_u8 *dst, const td_u8 *src,
        const td_i32 src_ch, 
        const td_i32 target_ch,
        td_i32 *conv_ch);

#endif
