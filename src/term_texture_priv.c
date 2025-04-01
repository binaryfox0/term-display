#include "term_texture_priv.h"
#include "term_priv.h"

#include <stdlib.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define SWAP(a, b) do { __typeof__(a) temp = a; a = b; b = temp; } while (0)

static inline void draw_pixel(u8 *texture,
                              const term_ivec2 size,
                              f32 *depth_buffer,
                              const term_ivec2 pos,
                              const f32 depth,
                              const u32 width,
                              const u8 color[4], const u8 ch, const u8 cch)
{
    if(!IN_RANGE(pos.x, 0, size.x -1 ) || !IN_RANGE(pos.y, 0, size.y - 1)) return;
    const u32 wpos = calculate_pos(pos.x, pos.y, width, 1);
    if (depth_buffer) {
        if (depth_buffer[wpos] <= depth)
            return;
        depth_buffer[wpos] = depth;
    }
    alpha_blend(&texture[wpos * ch], color, ch, cch);
}

void ptexture_draw_line(u8 *texture,
                        const term_ivec2 size,
                        const u8 channel,
                        const term_ivec2 p1,
                        const term_ivec2 p2,
                        const term_vec2 depth,
                        const term_rgba color, f32 *depth_buffer)
{
    u8 cch = 4, raw[4] = { 0 };
    convert(raw, (u8[4]) EXPAND_RGBA(color), channel, 4, &cch);

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
    float zStep = longLen ? (depth.y - depth.x) / longLen : 0;        // Depth interpolation step
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

static inline f32 edge_function(term_ivec2 v0, term_ivec2 v1, term_ivec2 v2) {
    return (f32)(v1.x - v0.x) * (v2.y - v0.y) - (f32)(v1.y - v0.y) * (v2.x - v0.x);
}

void ptexture_draw_triangle(u8 * texture,
    const term_ivec2 size,
    const u8 channel,
    const vertex v1,
    const vertex v2,
    const vertex v3,
    f32 * depth_buffer)
{
    vertex pv1 = v1, pv2 = v2, pv3 = v3;
    if(((pv2.pos.x - pv1.pos.x) * (pv3.pos.y - pv1.pos.y) - (pv2.pos.y - pv1.pos.y) * (pv3.pos.x - pv1.pos.x)) > 0)
        SWAP(pv2, pv3);

    
    int minX = max(0, min(v1.pos.x, min(v2.pos.x, v3.pos.x)));
    int minY = max(0, min(v1.pos.y, min(v2.pos.y, v3.pos.y)));
    int maxX = min(size.x - 1, max(v1.pos.x, max(v2.pos.x, v3.pos.x)));
    int maxY = min(size.y - 1, max(v1.pos.y, max(v2.pos.y, v3.pos.y)));

    int A0 = pv2.pos.y - pv3.pos.y, B0 = pv3.pos.x - pv2.pos.x, C0 = pv2.pos.x * pv3.pos.y - pv3.pos.x * pv2.pos.y;
    int A1 = pv3.pos.y - pv1.pos.y, B1 = pv1.pos.x - pv3.pos.x, C1 = pv3.pos.x * pv1.pos.y - pv1.pos.x * pv3.pos.y;
    int A2 = pv1.pos.y - pv2.pos.y, B2 = pv2.pos.x - pv1.pos.x, C2 = pv1.pos.x * pv2.pos.y - pv2.pos.x * pv1.pos.y;

    f32 inv_area = 1.0f / (B2 * A1 - (f32)(pv2.pos.y - pv1.pos.y) * (pv3.pos.x - pv1.pos.x));
    // /if(area == 0) return;

    for (int y = minY; y <= maxY; y++) {
        int w0_row = A0 * minX + B0 * y + C0;
        int w1_row = A1 * minX + B1 * y + C1;
        int w2_row = A2 * minX + B2 * y + C2;

        for (int x = minX; x <= maxX; x++) {
            // Compute barycentric coordinates
            float w0 = w0_row * inv_area;
            float w1 = w1_row * inv_area;
            float w2 = w2_row * inv_area;

            // Check if inside triangle
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                f32 pixel_depth = w0 * pv1.depth + w1 * pv2.depth + w2 * pv3.depth;

                // Interpolate color
                float r = w0 * pv1.color.r + w1 * pv2.color.r + w2 * pv3.color.r;
                float g = w0 * pv1.color.g + w1 * pv2.color.g + w2 * pv3.color.g;
                float b = w0 * pv1.color.b + w1 * pv2.color.b + w2 * pv3.color.b;
                float a = w0 * pv1.color.a + w1 * pv2.color.a + w2 * pv3.color.a;

                // Set pixel
                draw_pixel(texture, size, depth_buffer, ivec2_init(x, y), pixel_depth, size.x, (u8[4]){r, g, b, 255}, channel, 4);
            }
            w0_row += A0;
            w1_row += A1;
            w2_row += A2;
        }
    }
}