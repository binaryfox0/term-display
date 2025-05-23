#include "td_rasterizer.h"
#include "td_priv.h"

#include <stdlib.h>


#define SWAP(a, b) do { __typeof__(a) temp = a; a = b; b = temp; } while (0)

#define fast_floor(x) ((td_i32)(x))
#define fast_ceil(x) ((td_i32)(x) + ((x) > (td_i32)(x)))

TD_INLINE void draw_pixel(td_u8 *texture,
                              const td_ivec2 size,
                              td_f32 *depth_buffer,
                              const td_ivec2 pos,
                              const td_f32 depth,
                              const td_i32 width,
                              const td_u8 color[4], const td_u8 ch, const td_u8 cch)
{
    if(!IN_RANGE(pos.x, 0, size.x -1 ) || !IN_RANGE(pos.y, 0, size.y - 1)) return;
    const td_u64 wpos = calculate_pos(pos.x, pos.y, width, 1);
    if (depth_buffer) {
        if (depth_buffer[wpos] <= depth)
            return;
        depth_buffer[wpos] = depth;
    }
    alpha_blend(&texture[wpos * ch], color, ch, cch);
}

TD_INLINE td_u8 bilerp(td_u8 c00, td_u8 c10, td_u8 c01, td_u8 c11, float xt, float yt)
{
    return (td_u8) lerp(lerp(c00, c10, xt), lerp(c01, c11, xt), yt);
}

td_u8* ptexture_resize(const td_u8 *old, const td_u8 channel, const td_ivec2 old_size, const td_ivec2 new_size)
{
    float
        x_ratio = (float)(old_size.x - 1) / (float)(new_size.x - 1),
        y_ratio = (float)(old_size.y - 1) / (float)(new_size.y - 1);
    td_u8 *raw =
        (td_u8 *) malloc(calculate_pos(0, new_size.y, new_size.x, channel)),
        *start = raw;
    if (!raw)
        return 0;

    for (td_i32 row = 0; row < new_size.y; row++) {
        float tmp = (float)row * y_ratio;
        td_i32 iyf = fast_floor(tmp), iyc = fast_ceil(tmp);
        float ty = tmp - (td_f32)iyf;
        for (td_i32 col = 0; col < new_size.x; col++) {
            tmp = (float)col * x_ratio;
            td_i32 ixf = fast_floor(tmp), ixc = fast_ceil(tmp);
            float tx = tmp - (td_f32)ixf;

            td_u64 i00 = calculate_pos(ixf, iyf, old_size.x, channel),
                i10 = calculate_pos(ixc, iyf, old_size.x, channel),
                i01 = calculate_pos(ixf, iyc, old_size.x, channel),
                i11 = calculate_pos(ixc, iyc, old_size.x, channel);

            for (td_u8 c = 0; c < channel; c++, raw++)
                raw[0] =
                    bilerp(old[i00 + c], old[i10 + c], old[i01 + c],
                           old[i11 + c], tx, ty);
        }
    }
    return start;
}

void ptexture_draw_line(td_u8 *texture,
                        const td_ivec2 size,
                        const td_u8 channel,
                        const td_ivec2 p1,
                        const td_ivec2 p2,
                        const td_vec2 depth,
                        const td_rgba color, td_f32 *depth_buffer)
{
    td_u8 cch = 4, raw[4] = { 0 };
    convert(raw, (td_u8[4]) EXPAND_RGBA(color), channel, 4, &cch);

    int yLonger = 0;
    int incrementVal, endVal;
    int shortLen = p2.y - p1.y;
    int longLen = p2.x - p1.x;
    if (abs(shortLen) > abs(longLen)) {
        int tmp = shortLen;
        shortLen = longLen;
        longLen = tmp;
        yLonger = 1;
    }
    endVal = longLen;
    if (longLen < 0) {
        incrementVal = -1;
        longLen = -longLen;
    } else
        incrementVal = 1;
    int decInc = longLen ? (shortLen << 16) / longLen : 0;
    int j = 0;
    float zStep = longLen ? ((float)(depth.y - depth.x) / (float)longLen) : 0;        // Depth interpolation step
    float z = depth.x;
    if (yLonger) {
        for (int i = 0; i != endVal; i += incrementVal) {
            draw_pixel(texture,
                       size,
                       depth_buffer,
                       td_ivec2_init(p1.x + (j >> 16), p1.y + i), z,
                       size.x, raw, channel, cch);
            j += decInc;
            z += zStep;
        }
    } else {
        for (int i = 0; i != endVal; i += incrementVal) {
            draw_pixel(texture,
                       size,
                       depth_buffer,
                       td_ivec2_init(p1.x + i, p1.y + (j >> 16)), z,
                       size.x, raw, channel, cch);
            j += decInc;
            z += zStep;
        }
    }
}

TD_INLINE td_f32 edge_function(td_ivec2 v0, td_ivec2 v1, td_ivec2 v2) {
    return (td_f32)((v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x));
}

void ptexture_draw_triangle(
    td_u8 * output,
    const td_ivec2 size,
    const td_u8 channel,
    const vertex v1,
    const vertex v2,
    const vertex v3,
    td_f32 * depth_buffer,
    const td_u8* texture_data,
    const td_ivec2 texture_size
)
{
    vertex pv1 = v1, pv2 = v2, pv3 = v3;
    if(edge_function(pv1.pos, pv2.pos, pv3.pos) > 0)
        SWAP(pv2, pv3);
    
    td_u8 c_ch = 0;
    convert(pv1.color.raw, pv1.color.raw, channel, 4, &c_ch);
    convert(pv2.color.raw, pv2.color.raw, channel, 4, &c_ch);
    convert(pv3.color.raw, pv3.color.raw, channel, 4, &c_ch);

    // The bounding box
    int minX = max(0, min(v1.pos.x, min(v2.pos.x, v3.pos.x)));
    int minY = max(0, min(v1.pos.y, min(v2.pos.y, v3.pos.y)));
    int maxX = min(size.x - 1, max(v1.pos.x, max(v2.pos.x, v3.pos.x)));
    int maxY = min(size.y - 1, max(v1.pos.y, max(v2.pos.y, v3.pos.y)));

    int A0 = pv2.pos.y - pv3.pos.y, B0 = pv3.pos.x - pv2.pos.x, C0 = pv2.pos.x * pv3.pos.y - pv3.pos.x * pv2.pos.y;
    int A1 = pv3.pos.y - pv1.pos.y, B1 = pv1.pos.x - pv3.pos.x, C1 = pv3.pos.x * pv1.pos.y - pv1.pos.x * pv3.pos.y;
    int A2 = pv1.pos.y - pv2.pos.y, B2 = pv2.pos.x - pv1.pos.x, C2 = pv1.pos.x * pv2.pos.y - pv2.pos.x * pv1.pos.y;

    td_f32 area = edge_function(pv1.pos, pv2.pos, pv3.pos);
    if(area == 0) return;
    td_f32 inv_area = 1.0f / area;

    for (int y = minY; y <= maxY; y++) {
        int w0_row = A0 * minX + B0 * y + C0;
        int w1_row = A1 * minX + B1 * y + C1;
        int w2_row = A2 * minX + B2 * y + C2;

        for (int x = minX; x <= maxX; x++) {
            int covered = 0;
            td_f32 acc_r = 0.0f, acc_g = 0.0f, acc_b = 0.0f;
            // Compute barycentric coordinates
            float w0 = (float)w0_row * inv_area;
            float w1 = (float)w1_row * inv_area;
            float w2 = (float)w2_row * inv_area;

            // Check if inside triangle
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                td_f32 pixel_depth = w0 * pv1.depth + w1 * pv2.depth + w2 * pv3.depth;

                // Interpolate color
                td_u8 color[4] = {
                    (td_u8)(w0 * pv1.color.r + w1 * pv2.color.r + w2 * pv3.color.r),
                    (td_u8)(w0 * pv1.color.g + w1 * pv2.color.g + w2 * pv3.color.g),
                    (td_u8)(w0 * pv1.color.b + w1 * pv2.color.b + w2 * pv3.color.b),
                    (td_u8)(w0 * pv1.color.a + w1 * pv2.color.a + w2 * pv3.color.a)
                };

                td_f32 u = w0 * pv1.uv.x + w1 * pv2.uv.x + w2 * pv3.uv.x;
                td_f32 v = w0 * pv1.uv.y + w1 * pv2.uv.y + w2 * pv3.uv.y;


                // Set pixel
                draw_pixel(output, size, depth_buffer, td_ivec2_init(x, y), pixel_depth, size.x, color, channel, c_ch);
            }
            w0_row += A0;
            w1_row += A1;
            w2_row += A2;
        }
    }
}