#include "td_priv.h"
#include "td_main.h"

#include <stdio.h>
#include <string.h>

// // Convert b to have the same type as a
void tdp_convert_color(td_u8 *b_out, const td_u8 *b_in, td_i32 ch_a, td_i32 ch_b, td_i32 *out_b)
{
    td_u8 a_g = IS_GRAYSCALE(ch_a);
    td_u8 b_g = IS_GRAYSCALE(ch_b);
    td_i32 tmp;
    if(!out_b) { out_b = &tmp; }

    if (a_g && !b_g) { // A is grayscale, B is truecolor
        b_out[0] = to_grayscale(b_in); 
        b_out[1] = ch_b - 3 ? b_in[3] : 255;
        *out_b = ch_b - 2;
        return;
    } 
    else if (!a_g && b_g) { // A is truecolor, B is grayscale
        b_out[0] = b_out[1] = b_out[2] = b_in[0]; 
        b_out[3] = ch_b - 1 ? b_in[1] : 255;
        *out_b = ch_b + 2;
        return;
    }

    // If both are already the same format, copy directly
    for (td_u8 i = 0; i < ch_b; i++) {
        b_out[i] = b_in[i];
    }
    *out_b = ch_b;
}


// Color b (foreground) er color a (background)
void tdp_blend(td_u8 *a, const td_u8 *b, const td_i32 ch_a, const td_i32 ch_b)
{
    td_u8 out_a = IS_TRANSPARENT(ch_a);
    td_i32 a_i = ch_a - 1;
    td_i32 a_a = out_a ? a[a_i] : 255;
    td_u16 a_b = IS_TRANSPARENT(ch_b) ? b[ch_b - 1] : 255, iva_b = 255 - a_b;
    if (ch_a < 5)
        a[0] = (td_u8)((a_b * b[0] + iva_b * a[0]) >> 8);
    if (ch_a > 2) {
        a[1] = (td_u8)((a_b * b[1] + iva_b * a[1]) >> 8);
        a[2] = (td_u8)((a_b * b[2] + iva_b * a[2]) >> 8);
    }
    if (out_a)
    
    a[a_i] = (td_u8)(!iva_b ? 255 : a_b + ((iva_b + a_a) >> 8));
}

void tdp_fill_buffer(void* dest, const void* src, td_u64 destsz, td_u64 srcsz)
{
    if(!destsz || !srcsz || !dest || !src) return;
    if (destsz < srcsz) {
        memcpy(dest, src, destsz);
        return;
    }

    td_u8* ptr = (td_u8*)dest;
    memcpy(ptr, src, srcsz);

    size_t filled = srcsz;
    ptr += srcsz;

    while (filled * 2 <= destsz) {
        memcpy(ptr, dest, filled);
        ptr += filled;
        filled *= 2;
    }

    memcpy(ptr, dest, destsz - filled);
}
