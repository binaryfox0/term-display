#include "td_rasterizer.h"
#include "td_utils.h"
#include "td_texture_priv.h"

#include <stdlib.h>
#include <math.h>

TD_INLINE void tdp_perspective_division(
        const td_vec4 in,
        td_vec3 *out)
{
    out->x = in.x / in.w;
    out->y = in.y / in.w;
    out->z = in.z / in.w;
}

TD_INLINE void tdp_to_screen(
        const td_vec3 in,
        const td_ivec2 size,
        td_vec2 *out)
{
    out->x = (in.x + 1.0f) * 0.5f * (td_f32)size.x;
    out->y = (1.0f - in.y) * 0.5f * (td_f32)size.y;
}
// Change depth_buf to td_i32* to get pure integer execution
static inline void tdp_rasterize_pixel_fixed(
                                const td_texture_t* fb,
                                td_i32* depth_buf, 
                                const td_ivec2 pos,
                                const td_i32 depth, // Pure 16.16 fixed depth
                                const td_u8 color[4])
{
    // Fast boundary check: compiler optimizes this easily
    if (pos.x < 0 || pos.x >= fb->size.x || pos.y < 0 || pos.y >= fb->size.y)
        return;

    td_u64 pixel_pos = tdp_calculate_pos(pos, fb->size.x, 1);
    
    if (depth_buf) 
    {
        // Pure integer comparison (Extremely Fast)
        if (depth_buf[pixel_pos] <= depth)
            return;
        depth_buf[pixel_pos] = depth;
    }

    td_u8 *pixel_ptr = fb->data + pixel_pos * (td_u64)fb->type; 
    tdp_blend(pixel_ptr, color, (td_i32)fb->type, 4, pixel_ptr);
}

void tdp_rasterize_line(const td_texture_t* fb,
                        td_i32* depth_buf, // Max performance requires int depth buffer
                        const tdp_vertex_t v0,
                        const tdp_vertex_t v1)
{
    td_vec3 ndc0, ndc1;
    td_vec2 s0, s1;
   
    tdp_perspective_division(v0.pos, &ndc0); 
    tdp_perspective_division(v1.pos, &ndc1); 
    tdp_to_screen(ndc0, fb->size, &s0);
    tdp_to_screen(ndc1, fb->size, &s1);

    td_i32 x0 = (td_i32)s0.x;
    td_i32 y0 = (td_i32)s0.y;
    td_i32 x1 = (td_i32)s1.x;
    td_i32 y1 = (td_i32)s1.y;

    td_i32 dx = x1 - x0;
    td_i32 dy = y1 - y0;

    td_i32 abs_dx = abs(dx);
    td_i32 abs_dy = abs(dy);

    if (abs_dx == 0 && abs_dy == 0) return;

    td_i32 depth_start = (td_i32)(v0.depth * 65536.0f);
    td_i32 depth_end   = (td_i32)(v1.depth * 65536.0f);
    
    td_i32 r_start = v0.color.r << 16;
    td_i32 g_start = v0.color.g << 16;
    td_i32 b_start = v0.color.b << 16;
    td_i32 a_start = v0.color.a << 16;

    td_i32 depth_diff = depth_end - depth_start;
    td_i32 r_diff     = (v1.color.r - v0.color.r) << 16;
    td_i32 g_diff     = (v1.color.g - v0.color.g) << 16;
    td_i32 b_diff     = (v1.color.b - v0.color.b) << 16;
    td_i32 a_diff     = (v1.color.a - v0.color.a) << 16;

    td_u8 active_color[4];

    if (abs_dx >= abs_dy) 
    {
        if (x0 > x1) {
            td_i32 t = x0; x0 = x1; x1 = t;
            t = y0; y0 = y1; y1 = t;
            depth_start = depth_end; depth_diff = -depth_diff;
            r_start = r_start + r_diff; r_diff = -r_diff;
            g_start = g_start + g_diff; g_diff = -g_diff;
            b_start = b_start + b_diff; b_diff = -b_diff;
            a_start = a_start + a_diff; a_diff = -a_diff;
            dx = -dx; dy = -dy;
        }

        td_i32 dec_inc = (dy << 16) / dx; 
        td_i32 j = y0 << 16;

        td_i32 depth_step = depth_diff / dx;
        td_i32 r_step     = r_diff / dx;
        td_i32 g_step     = g_diff / dx;
        td_i32 b_step     = b_diff / dx;
        td_i32 a_step     = a_diff / dx;

        for (td_i32 x = x0; x <= x1; ++x) 
        {
            active_color[0] = (td_u8)(r_start >> 16);
            active_color[1] = (td_u8)(g_start >> 16);
            active_color[2] = (td_u8)(b_start >> 16);
            active_color[3] = (td_u8)(a_start >> 16);

            tdp_rasterize_pixel_fixed(fb, depth_buf,
                       (td_ivec2){.x = x, .y = j >> 16}, 
                       depth_start, active_color);

            j += dec_inc;
            depth_start += depth_step;
            r_start += r_step; g_start += g_step; b_start += b_step; a_start += a_step;
        }
    } 
    else 
    {
        if (y0 > y1) {
            td_i32 t = x0; x0 = x1; x1 = t;
            t = y0; y0 = y1; y1 = t;
            depth_start = depth_end; depth_diff = -depth_diff;
            r_start = r_start + r_diff; r_diff = -r_diff;
            g_start = g_start + g_diff; g_diff = -g_diff;
            b_start = b_start + b_diff; b_diff = -b_diff;
            a_start = a_start + a_diff; a_diff = -a_diff;
            dx = -dx; dy = -dy;
        }

        td_i32 dec_inc = (dx << 16) / dy; 
        td_i32 j = x0 << 16;

        td_i32 depth_step = depth_diff / dy;
        td_i32 r_step     = r_diff / dy;
        td_i32 g_step     = g_diff / dy;
        td_i32 b_step     = b_diff / dy;
        td_i32 a_step     = a_diff / dy;

        for (td_i32 y = y0; y <= y1; ++y) 
        {
            active_color[0] = (td_u8)(r_start >> 16);
            active_color[1] = (td_u8)(g_start >> 16);
            active_color[2] = (td_u8)(b_start >> 16);
            active_color[3] = (td_u8)(a_start >> 16);

            tdp_rasterize_pixel_fixed(fb, depth_buf,
                       (td_ivec2){.x = j >> 16, .y = y}, 
                       depth_start, active_color);

            j += dec_inc;
            depth_start += depth_step;
            r_start += r_step; g_start += g_step; b_start += b_step; a_start += a_step;
        }
    }
}

/*
TD_INLINE td_f32 tdp_edge_function(
        const td_ivec2 v0, 
        const td_ivec2 v1, 
        const td_ivec2 v2) 
{
    return (td_f32)((v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x));
}*/
/*
void tdp_rasterize_line(const td_texture_t* fb,
                        td_f32* depth_buf,
                        const tdp_vertex_t v0,
                        const tdp_vertex_t v1)
{
    static td_rgba default_color = {.r = 255, .g = 255, .b = 255, .a = 255};

    td_vec3 ndc0 = {0}, ndc1 = {0};
    td_vec2 s0 = {0}, s1 = {0};
    td_bool y_longer = TD_FALSE;
    td_i32 inc_val = 0, end_val = 0;
    td_i32 short_len = 0, long_len = 0;
    td_i32 dec_inc = 0;
    td_i32 j = 0;
   
    tdp_perspective_division(v0.pos, &ndc0); 
    tdp_perspective_division(v1.pos, &ndc1); 
    tdp_to_screen(ndc0, fb->size, &s0);
    tdp_to_screen(ndc0, fb->size, &s1);
    long_len = (td_i32)(s1.x - s0.x);
    short_len = (td_i32)(s1.y - s0.y);
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
                       (td_ivec2){
                            .x=(td_i32)s0.x + (j >> 16), 
                            .y=(td_i32)s0.y + i}, 
                       tdp_lerp(v0.depth, v1.depth, t),
                       default_color.raw);
            j += dec_inc;
        }
    } else {
        for (int i = 0; i != end_val; i += inc_val) 
        {
            td_f32 t = (td_f32)i / (td_f32)end_val;
            tdp_rasterize_pixel(fb, depth_buf,
                       (td_ivec2){
                            .x=(td_i32)s0.x + i, 
                            .y=(td_i32)s0.y + (j >> 16)}, 
                       tdp_lerp(v0.depth, v1.depth, t),
                       default_color.raw);
            j += dec_inc;
        }
    }
}
*/


void tdp_rasterize_triangle(
    const td_texture_t* fb,
    td_f32* depth_buf,
    const tdp_vertex_t v0,
    const tdp_vertex_t v1,
    const tdp_vertex_t v2,
    const td_texture_t* tex
)
{
    (void)depth_buf;
    td_vec3 ndc0 = {0}, ndc1 = {0}, ndc2 = {0};
    td_vec2 s0 = {0}, s1 = {0}, s2 = {0};
    if(
            v0.pos.w <= 0.0f || 
            v1.pos.w <= 0.0f || 
            v2.pos.w <= 0.0f) 
        return;

    tdp_perspective_division(v0.pos, &ndc0);
    tdp_perspective_division(v1.pos, &ndc1);
    tdp_perspective_division(v2.pos, &ndc2);

    tdp_to_screen(ndc0, fb->size, &s0);
    tdp_to_screen(ndc1, fb->size, &s1);
    tdp_to_screen(ndc2, fb->size, &s2);

    // Setup local vertex structures with screen space positions for edge equations
    // We also store 1/w for perspective-correct interpolation
    tdp_vertex_t vv0 = v0;
    tdp_vertex_t vv1 = v1;
    tdp_vertex_t vv2 = v2;

    td_f32 inv_w0 = 0.0f;
    td_f32 inv_w1 = 0.0f;
    td_f32 inv_w2 = 0.0f;

    inv_w0 = 1.0f / vv0.pos.w;
    inv_w1 = 1.0f / vv1.pos.w;
    inv_w2 = 1.0f / vv2.pos.w;

    td_f32 area = 0.0f, inv_area = 0.0f;
    td_i32 min_x = 0, min_y = 0, max_x = 0, max_y = 0;
    td_f32 A0 = 0, B0 = 0, C0 = 0;
    td_f32 A1 = 0, B1 = 0, C1 = 0;
    td_f32 A2 = 0, B2 = 0, C2 = 0;

    td_u8 *tex_data = 0;
    td_ivec2 tex_size = (td_ivec2){0};
    td_i32 tex_ch = 0;

    // Edge function using floating-point screen coordinates for accuracy
    //area = (s2.x - s0.x) * (s1.y - s0.y) - (s2.y - s0.y) * (s1.x - s0.x);
    area = (s1.x - s0.x) * (s2.y - s0.y)
     - (s1.y - s0.y) * (s2.x - s0.x);
    // Backface culling / Winding order check
    if (area > 0)
    {
        TDP_SWAP(s1, s2, td_vec2);
        TDP_SWAP(vv1, vv2, tdp_vertex_t);
        TDP_SWAP(inv_w1, inv_w2, td_f32);

        area = -area;
    }
    if (fabs(area) < 0.00001f) return; // Degenerate triangle
    
    inv_area = 1.0f / area;
    
    // Bounding box (clamped to framebuffer)
    min_x = TDP_MAX(0, (td_i32)tdp_floor(TDP_MIN(s0.x, TDP_MIN(s1.x, s2.x))));
    min_y = TDP_MAX(0, (td_i32)tdp_floor(TDP_MIN(s0.y, TDP_MIN(s1.y, s2.y))));
    max_x = TDP_MIN(fb->size.x - 1, (td_i32)tdp_ceil(TDP_MAX(s0.x, TDP_MAX(s1.x, s2.x))));
    max_y = TDP_MIN(fb->size.y - 1, (td_i32)tdp_ceil(TDP_MAX(s0.y, TDP_MAX(s1.y, s2.y))));

    // Edge coefficients updated to float to accommodate sub-pixel accuracy
    A0 = s1.y - s2.y; B0 = s2.x - s1.x; C0 = s1.x * s2.y - s2.x * s1.y;
    A1 = s2.y - s0.y; B1 = s0.x - s2.x; C1 = s2.x * s0.y - s0.x * s2.y;
    A2 = s0.y - s1.y; B2 = s1.x - s0.x; C2 = s0.x * s1.y - s1.x * s0.y;

    if(tex)
    {
        tex_data = tex->data;
        tex_size = tex->size;
        tex_ch = (td_i32)tex->type;
    }

    for (int y = min_y; y <= max_y; y++) 
    {
        // Use float evaluation for sub-pixel accuracy
        td_f32 w0_row = A0 * (td_f32)min_x + B0 * (td_f32)y + C0;
        td_f32 w1_row = A1 * (td_f32)min_x + B1 * (td_f32)y + C1;
        td_f32 w2_row = A2 * (td_f32)min_x + B2 * (td_f32)y + C2;

        for (int x = min_x; x <= max_x; x++) 
        {
            // Compute screening space barycentric coordinates
            td_f32 w0 = w0_row * inv_area;
            td_f32 w1 = w1_row * inv_area;
            td_f32 w2 = w2_row * inv_area;

            // Check if inside triangle (using a tiny epsilon for float precision edge cases)
            if (w0 >= -0.00001f && w1 >= -0.00001f && w2 >= -0.00001f) {
                
                // 2. Perspective-Correct Interpolation
                // Interpolate 1/w first
                td_f32 interpolated_inv_w = w0 * inv_w0 + w1 * inv_w1 + w2 * inv_w2;
                td_f32 current_w = 1.0f / interpolated_inv_w;

                // Correct barycentric coordinates for 3D attributes
                td_f32 p_w0 = w0 * inv_w0 * current_w;
                td_f32 p_w1 = w1 * inv_w1 * current_w;
                td_f32 p_w2 = w2 * inv_w2 * current_w;

                // Interpolate depth (Z) using NDC Z or View space depth
                td_f32 pix_depth = p_w0 * ndc0.z + p_w1 * ndc1.z + p_w2 * ndc2.z;

                // Interpolate vertex color
                td_u8 final_color[4] = {
                    (td_u8)(p_w0 * vv0.color.r + p_w1 * vv1.color.r + p_w2 * vv2.color.r),
                    (td_u8)(p_w0 * vv0.color.g + p_w1 * vv1.color.g + p_w2 * vv2.color.g),
                    (td_u8)(p_w0 * vv0.color.b + p_w1 * vv1.color.b + p_w2 * vv2.color.b),
                    (td_u8)(p_w0 * vv0.color.a + p_w1 * vv1.color.a + p_w2 * vv2.color.a)
                };
                
                if (tex_data) 
                {
                    td_f32 u = 0.0f, v = 0.0f;
                    td_u8 texel[4] = {0};

                    // Perspective correct UV coordinates
                    u = p_w0 * vv0.uv.x + p_w1 * vv1.uv.x + p_w2 * vv2.uv.x;
                    v = p_w0 * vv0.uv.y + p_w1 * vv1.uv.y + p_w2 * vv2.uv.y;

                    td_f32 tex_u = u * (float)(tex_size.x - 1);
                    td_f32 tex_v = (1.0f - v) * (float)(tex_size.y - 1);

                    td_i32 ixf = tdp_floor(tex_u);
                    td_i32 ixc = TDP_MIN(ixf + 1, tex_size.x - 1);
                    td_i32 iyf = tdp_floor(tex_v);
                    td_i32 iyc = TDP_MIN(iyf + 1, tex_size.y - 1);

                    td_f32 tx = tex_u - (td_f32)ixf;
                    td_f32 ty = tex_v - (td_f32)iyf;

                    td_u64 i00 = tdp_calculate_pos((td_ivec2){.x=ixf, .y=iyf}, tex_size.x, tex_ch);
                    td_u64 i10 = tdp_calculate_pos((td_ivec2){.x=ixc, .y=iyf}, tex_size.x, tex_ch);
                    td_u64 i01 = tdp_calculate_pos((td_ivec2){.x=ixf, .y=iyc}, tex_size.x, tex_ch);
                    td_u64 i11 = tdp_calculate_pos((td_ivec2){.x=ixc, .y=iyc}, tex_size.x, tex_ch);

                    for (td_u8 c = 0; c < tex_ch; c++)
                        texel[c] = bilerp(tex_data[i00 + c], tex_data[i10 + c], tex_data[i01 + c], tex_data[i11 + c], tx, ty);
                    
                    tdp_blend(final_color, texel, 4, tex_ch, final_color);
                }
(void)pix_depth;
                // Set pixel
                tdp_rasterize_pixel_fixed(
                        fb, NULL, 
                        (td_ivec2){.x=x,.y=y}, 
                        0, 
                        final_color);
            }
            w0_row += A0;
            w1_row += A1;
            w2_row += A2;
        }
    }
}
