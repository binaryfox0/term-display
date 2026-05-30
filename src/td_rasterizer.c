#include "td_rasterizer.h"
#include "td_utils.h"
#include "td_texture_priv.h"

#include <stdlib.h>

static inline void tdp_rasterize_pixel(
                                const td_texture_t* fb,
                                td_f32* depth_buf,
                                const td_ivec2 pos,
                                const td_f32 depth,
                                const td_u8 color[4])
{
    if(!IN_RANGE(pos.x, 0, fb->size.x -1 ) || !IN_RANGE(pos.y, 0, fb->size.y - 1)) return;
    const td_u64 wpos = tdp_calculate_pos(pos, fb->size.x, 1);
    if (depth_buf) {
        if (depth_buf[wpos] <= depth)
            return;
        depth_buf[wpos] = depth;
    }
    tdp_blend(fb->data + (wpos * (td_u64)fb->type), color, 
            (td_i32)fb->type, 4);
}

void tdp_rasterize_line(const td_texture_t* fb,
                        td_f32* depth_buf,
                        const tdp_vertex_t v0,
                        const tdp_vertex_t v1)
{
    static td_rgba default_color = {.r = 255, .g = 255, .b = 255, .a = 255};

    td_bool y_longer = TD_FALSE;
    td_i32 inc_val = 0, end_val = 0;
    td_i32 short_len = 0, long_len = 0;
    td_i32 dec_inc = 0;
    td_i32 j = 0;
    
    long_len = v1.pos.x - v0.pos.x;
    short_len = v1.pos.y - v0.pos.y;
    if (abs(short_len) > abs(long_len)) 
    {
        int tmp = short_len;
        short_len = long_len;
        long_len = tmp;
        y_longer = TD_TRUE;
    }
    end_val = long_len;
    if (long_len < 0) {
        inc_val = -1;
        long_len = -long_len;
    } else
        inc_val = 1;
    dec_inc = long_len ? (short_len << 16) / long_len : 0;
    if (y_longer) 
    {
        for (int i = 0; i != end_val; i += inc_val) 
        {
            td_f32 t = (td_f32)i / (td_f32)end_val;
            tdp_rasterize_pixel(fb, depth_buf,
                       (td_ivec2){.x=v0.pos.x + (j >> 16), .y=v0.pos.y + i}, 
                       tdp_lerp(v0.depth, v1.depth, t),
                       default_color.raw);
            j += dec_inc;
        }
    } else {
        for (int i = 0; i != end_val; i += inc_val) 
        {
            td_f32 t = (td_f32)i / (td_f32)end_val;
            tdp_rasterize_pixel(fb, depth_buf,
                       (td_ivec2){.x=v0.pos.x + i, .y=v0.pos.y + (j >> 16)}, 
                       tdp_lerp(v0.depth, v1.depth, t),
                       default_color.raw);
            j += dec_inc;
        }
    }
}

TD_INLINE td_f32 tdp_edge_function(
        const td_ivec2 v0, 
        const td_ivec2 v1, 
        const td_ivec2 v2) 
{
    return (td_f32)((v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x));
}

void tdp_rasterize_triangle(
    const td_texture_t* fb,
    td_f32* depth_buf,
    const tdp_vertex_t v0,
    const tdp_vertex_t v1,
    const tdp_vertex_t v2,
    const td_texture_t* tex
)
{
    tdp_vertex_t pv0 = v0, pv1 = v1, pv2 = v2;

    td_f32 area = 0.0f, inv_area = 0.0f;
    td_i32 min_x = 0, min_y = 0, max_x = 0, max_y = 0;
    td_i32 A0 = 0, B0 = 0, C0 = 0;
    td_i32 A1 = 0, B1 = 0, C1 = 0;
    td_i32 A2 = 0, B2 = 0, C2 = 0;

    td_u8 *tex_data = 0;
    td_ivec2 tex_size = (td_ivec2){0};
    td_i32 tex_ch = 0;

    area = tdp_edge_function(pv0.pos, pv1.pos, pv2.pos);
    if(area > 0)
    {
        TDP_SWAP(pv1, pv2, tdp_vertex_t);
        area = -area;
        // return;
    }
    inv_area = 1.0f / area;
    
    // The bounding box
    min_x = tdp_max(0, tdp_min(v0.pos.x, tdp_min(v1.pos.x, v2.pos.x)));
    min_y = tdp_max(0, tdp_min(v0.pos.y, tdp_min(v1.pos.y, v2.pos.y)));
    max_x = tdp_min(fb->size.x - 1, tdp_max(v0.pos.x, tdp_max(v1.pos.x, v2.pos.x)));
    max_y = tdp_min(fb->size.y - 1, tdp_max(v0.pos.y, tdp_max(v1.pos.y, v2.pos.y)));

    A0 = pv1.pos.y - pv2.pos.y; 
    B0 = pv2.pos.x - pv1.pos.x; 
    C0 = pv1.pos.x * pv2.pos.y - pv2.pos.x * pv1.pos.y;
    A1 = pv2.pos.y - pv0.pos.y; 
    B1 = pv0.pos.x - pv2.pos.x; 
    C1 = pv2.pos.x * pv0.pos.y - pv0.pos.x * pv2.pos.y;
    A2 = pv0.pos.y - pv1.pos.y; 
    B2 = pv1.pos.x - pv0.pos.x; 
    C2 = pv0.pos.x * pv1.pos.y - pv1.pos.x * pv0.pos.y;

    if(tex)
    {
        tex_data = tex->data;
        tex_size = tex->size;
        tex_ch = (td_i32)tex->type;
    }

    for (int y = min_y; y <= max_y; y++) 
    {
        int w0_row = A0 * min_x + B0 * y + C0;
        int w1_row = A1 * min_x + B1 * y + C1;
        int w2_row = A2 * min_x + B2 * y + C2;

        for (int x = min_x; x <= max_x; x++) 
        {
            // Compute barycentric coordinates
            float w0 = (float)w0_row * inv_area;
            float w1 = (float)w1_row * inv_area;
            float w2 = (float)w2_row * inv_area;

            // Check if inside triangle
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                td_f32 pix_depth = w0 * pv0.depth + w1 * pv1.depth + w2 * pv2.depth;

                // Interpolate color
                td_u8 final_color[4] = {
                    (td_u8)(w0 * pv0.color.r + w1 * pv1.color.r + w2 * pv2.color.r),
                    (td_u8)(w0 * pv0.color.g + w1 * pv1.color.g + w2 * pv2.color.g),
                    (td_u8)(w0 * pv0.color.b + w1 * pv1.color.b + w2 * pv2.color.b),
                    (td_u8)(w0 * pv0.color.a + w1 * pv1.color.a + w2 * pv2.color.a)
                };
                
                if (tex_data) 
                {
                    td_f32 u = 0.0f, v = 0.0f;
                    td_u8 texel[4] = {0};

                    u = w0 * pv0.uv.x + w1 * pv1.uv.x + w2 * pv2.uv.x;
                    v = w0 * pv0.uv.y + w1 * pv1.uv.y + w2 * pv2.uv.y;

                    td_f32 tex_u = u * (float)(tex_size.x - 1);
                    td_f32 tex_v = (1.0f - v) * (float)(tex_size.y - 1);

                    td_i32 ixf = tdp_floor(tex_u);
                    td_i32 ixc = tdp_min(ixf + 1, tex_size.x - 1);
                    td_i32 iyf = tdp_floor(tex_v);
                    td_i32 iyc = tdp_min(iyf + 1, tex_size.y - 1);

                    td_f32 tx = tex_u - (td_f32)ixf;
                    td_f32 ty = tex_v - (td_f32)iyf;

                    td_u64 i00 = tdp_calculate_pos((td_ivec2){.x=ixf, .y=iyf}, 
                            tex_size.x, tex_ch);
                    td_u64 i10 = tdp_calculate_pos((td_ivec2){.x=ixc, .y=iyf}, 
                            tex_size.x, tex_ch);
                    td_u64 i01 = tdp_calculate_pos((td_ivec2){.x=ixf, .y=iyc}, 
                            tex_size.x, tex_ch);
                    td_u64 i11 = tdp_calculate_pos((td_ivec2){.x=ixc, .y=iyc}, 
                            tex_size.x, tex_ch);

                    for (td_u8 c = 0; c < tex_ch; c++)
                        texel[c] = bilerp(
                            tex_data[i00 + c],
                            tex_data[i10 + c],
                            tex_data[i01 + c],
                            tex_data[i11 + c],
                            tx, ty
                        );
                    
                    tdp_blend(final_color, texel, 4, tex_ch);
                }

                // Set pixel
                tdp_rasterize_pixel(fb, depth_buf, (td_ivec2){.x=x,.y=y}, pix_depth, final_color);
            }
            w0_row += A0;
            w1_row += A1;
            w2_row += A2;
        }
    }
}
