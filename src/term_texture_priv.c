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
    convert(raw, (u8[4]) EXPAND_RGBA(color), channel, &cch);

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

void ptexture_draw_triangle(
    u8 *texture,
    const term_ivec2 size,
    const u8 channel,
    const term_ivec2 p1,
    const term_ivec2 p2,
    const term_ivec2 p3,
    const term_rgba color,
    const term_vec3 depth,
    f32 *depth_buffer
) {
    term_ivec2 v1 = p1, v2 = p2, v3 = p3;
    float d1 = depth.x, d2 = depth.y, d3 = depth.z;

    // Ensure CCW winding order
    int area = (v2.x - v1.x) * (v3.y - v1.y) - (v3.x - v1.x) * (v2.y - v1.y);
    if (area < 0) {
        SWAP(v2, v3);
        SWAP(d2, d3);
    }

    u8 cch = 4, raw[4] = {0};
    convert(raw, (u8[4])EXPAND_RGBA(color), channel, &cch);

    // Compute triangle bounding box
    int minX = max(0, min(v1.x, min(v2.x, v3.x)));
    int maxX = min(size.x - 1, max(v1.x, max(v2.x, v3.x)));
    int minY = min(v1.y, min(v2.y, v3.y));
    int maxY = max(v1.y, max(v2.y, v3.y));

    int A0 = v2.y - v3.y, B0 = v3.x - v2.x, C0 = v2.x * v3.y - v3.x * v2.y;
    int A1 = v3.y - v1.y, B1 = v1.x - v3.x, C1 = v3.x * v1.y - v1.x * v3.y;
    int A2 = v1.y - v2.y, B2 = v2.x - v1.x, C2 = v1.x * v2.y - v2.x * v1.y;

    float invSum = 1.0f / ((A0 * v1.x + B0 * v1.y + C0) + 
                            (A1 * v2.x + B1 * v2.y + C1) + 
                            (A2 * v3.x + B2 * v3.y + C2));

    for (int y = minY; y <= maxY; y++) {
        int w0_row = A0 * minX + B0 * y + C0;
        int w1_row = A1 * minX + B1 * y + C1;
        int w2_row = A2 * minX + B2 * y + C2;

        for (int x = minX; x <= maxX; x++) {
            if ((w0_row | w1_row | w2_row) >= 0) {
                // Convert to barycentric coordinates
                float alpha = w0_row * invSum;
                float beta = w1_row * invSum;
                float gamma = w2_row * invSum;

                float z = alpha * d1 + beta * d2 + gamma * d3;
                
                draw_pixel(texture, size, depth_buffer, ivec2_init(x, y), z, size.x, raw, channel, cch);
            }
            
            w0_row += A0;
            w1_row += A1;
            w2_row += A2;
        }
    }
}
