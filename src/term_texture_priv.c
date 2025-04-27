#include "term_texture_priv.h"
#include "term_priv.h"

#include <stdlib.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define SWAP(a, b) do { __typeof__(a) temp = a; a = b; b = temp; } while (0)

#define fast_floor(x) ((term_i32)(x))
#define fast_ceil(x) ((term_i32)(x) + ((x) > (term_i32)(x)))

TD_INLINE void draw_pixel(term_u8 *texture,
                              const term_ivec2 size,
                              term_f32 *depth_buffer,
                              const term_ivec2 pos,
                              const term_f32 depth,
                              const term_i32 width,
                              const term_u8 color[4], const term_u8 ch, const term_u8 cch)
{
    if(!IN_RANGE(pos.x, 0, size.x -1 ) || !IN_RANGE(pos.y, 0, size.y - 1)) return;
    const term_u64 wpos = calculate_pos(pos.x, pos.y, width, 1);
    if (depth_buffer) {
        if (depth_buffer[wpos] <= depth)
            return;
        depth_buffer[wpos] = depth;
    }
    alpha_blend(&texture[wpos * ch], color, ch, cch);
}

TD_INLINE term_u8 bilerp(term_u8 c00, term_u8 c10, term_u8 c01, term_u8 c11, float xt, float yt)
{
    return (term_u8) lerp(lerp(c00, c10, xt), lerp(c01, c11, xt), yt);
}

term_u8* ptexture_resize(const term_u8 *old, const term_u8 channel, const term_ivec2 old_size, const term_ivec2 new_size)
{
    float
        x_ratio = (float)(old_size.x - 1) / (float)(new_size.x - 1),
        y_ratio = (float)(old_size.y - 1) / (float)(new_size.y - 1);
    term_u8 *raw =
        (term_u8 *) malloc(calculate_pos(0, new_size.y, new_size.x, channel)),
        *start = raw;
    if (!raw)
        return 0;

    for (term_i32 row = 0; row < new_size.y; row++) {
        float tmp = (float)row * y_ratio;
        term_i32 iyf = fast_floor(tmp), iyc = fast_ceil(tmp);
        float ty = tmp - (term_f32)iyf;
        for (term_i32 col = 0; col < new_size.x; col++) {
            tmp = (float)col * x_ratio;
            term_i32 ixf = fast_floor(tmp), ixc = fast_ceil(tmp);
            float tx = tmp - (term_f32)ixf;

            term_u64 i00 = calculate_pos(ixf, iyf, old_size.x, channel),
                i10 = calculate_pos(ixc, iyf, old_size.x, channel),
                i01 = calculate_pos(ixf, iyc, old_size.x, channel),
                i11 = calculate_pos(ixc, iyc, old_size.x, channel);

            for (term_u8 c = 0; c < channel; c++, raw++)
                raw[0] =
                    bilerp(old[i00 + c], old[i10 + c], old[i01 + c],
                           old[i11 + c], tx, ty);
        }
    }
    return start;
}

void ptexture_draw_line(term_u8 *texture,
                        const term_ivec2 size,
                        const term_u8 channel,
                        const term_ivec2 p1,
                        const term_ivec2 p2,
                        const term_vec2 depth,
                        const term_rgba color, term_f32 *depth_buffer)
{
    term_u8 cch = 4, raw[4] = { 0 };
    convert(raw, (term_u8[4]) EXPAND_RGBA(color), channel, 4, &cch);

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
                       ivec2_init(p1.x + (j >> 16), p1.y + i), z,
                       size.x, raw, channel, cch);
            j += decInc;
            z += zStep;
        }
    } else {
        for (int i = 0; i != endVal; i += incrementVal) {
            draw_pixel(texture,
                       size,
                       depth_buffer,
                       ivec2_init(p1.x + i, p1.y + (j >> 16)), z,
                       size.x, raw, channel, cch);
            j += decInc;
            z += zStep;
        }
    }
}

TD_INLINE term_f32 edge_function(term_ivec2 v0, term_ivec2 v1, term_ivec2 v2) {
    return (term_f32)((v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x));
}

void ptexture_draw_triangle(term_u8 * texture,
    const term_ivec2 size,
    const term_u8 channel,
    const vertex v1,
    const vertex v2,
    const vertex v3,
    term_f32 * depth_buffer,
    const term_u8* texture_data)
{
    vertex pv1 = v1, pv2 = v2, pv3 = v3;
    if(((pv2.pos.x - pv1.pos.x) * (pv3.pos.y - pv1.pos.y) - (pv2.pos.y - pv1.pos.y) * (pv3.pos.x - pv1.pos.x)) > 0)
        SWAP(pv2, pv3);
    
    term_u8 c_ch = 0;
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

    term_f32 inv_area = 1.0f / (float)(B2 * A1 - (pv2.pos.y - pv1.pos.y) * (pv3.pos.x - pv1.pos.x));
    // /if(area == 0) return;

    for (int y = minY; y <= maxY; y++) {
        int w0_row = A0 * minX + B0 * y + C0;
        int w1_row = A1 * minX + B1 * y + C1;
        int w2_row = A2 * minX + B2 * y + C2;

        for (int x = minX; x <= maxX; x++) {
            int covered = 0;
            term_f32 acc_r = 0.0f, acc_g = 0.0f, acc_b = 0.0f;
            // Compute barycentric coordinates
            float w0 = (float)w0_row * inv_area;
            float w1 = (float)w1_row * inv_area;
            float w2 = (float)w2_row * inv_area;

            // Check if inside triangle
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                term_f32 pixel_depth = w0 * pv1.depth + w1 * pv2.depth + w2 * pv3.depth;

                // Interpolate color
                term_u8 color[4] = {
                    (term_u8)(w0 * pv1.color.r + w1 * pv2.color.r + w2 * pv3.color.r),
                    (term_u8)(w0 * pv1.color.g + w1 * pv2.color.g + w2 * pv3.color.g),
                    (term_u8)(w0 * pv1.color.b + w1 * pv2.color.b + w2 * pv3.color.b),
                    (term_u8)(w0 * pv1.color.a + w1 * pv2.color.a + w2 * pv3.color.a)
                };

                // Set pixel
                draw_pixel(texture, size, depth_buffer, ivec2_init(x, y), pixel_depth, size.x, color, channel, c_ch);
            }
            w0_row += A0;
            w1_row += A1;
            w2_row += A2;
        }
    }
}