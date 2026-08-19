#include "swrz_pool.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "swrz_utils.h"
#include "swrz_alloc_priv.h"
#include "swrz_thread.h"
#include "swrz_mutex.h"
#include "swrz_condvar.h"
#include "softrast/swrz_vertex.h"
#include "swrz_texture_priv.h"

static void swrz__pool_thread(
        void *param)
{
    swrz__pool_t *pool = param;
    swrz__mutex_lock(pool->mutex);
    for(;;)
    {
        if(swrz__condvar_wait(
                    pool->work_cv,
                    pool->mutex) != SWRZ_ERR_OK)
            break;
        if(swrz__atomic_load(&pool->abort))
            break;
        
        swrz__mutex_unlock(pool->mutex);

        swrz__mutex_lock(pool->mutex);
    }
    swrz__mutex_unlock(pool->mutex);
}

swrz_error_t swrz__pool_init(
        swrz__pool_t *pool)
{
    swrz_error_t ret = SWRZ_ERR_OK;
    if(!pool)
        return SWRZ_ERR_PARAM;

    pool->thread_count = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if(pool->thread_count <= 0)
        pool->thread_count = 1;

    SWRZ__CHECK(swrz__mutex_create(&pool->mutex), ret, fail);
    SWRZ__CHECK(swrz__condvar_create(&pool->work_cv), ret, fail);

    swrz__atomic_store(&pool->abort, 0);
    swrz__arena_reset(&pool->arena);
    pool->threads = swrz__calloc(
            (size_t)pool->thread_count, 
            sizeof(*pool->threads));
    if(!pool->threads)
        { ret = SWRZ_ERR_NO_MEM; goto fail; }
    for(int i = 0; i < pool->thread_count; i++)
    {
        SWRZ__CHECK(
                swrz__thread_create(
                    &pool->threads[i], 
                    swrz__pool_thread, pool),
                ret, fail);
    }

    swrz__arena_reset(&pool->arena);

    return SWRZ_ERR_OK;

fail:
    swrz__pool_destroy(pool);
    return ret;
}

#define SWRZ__FIXED_FRAC_BITS 8
#define SWRZ__FIXED_ONE (1 << SWRZ__FIXED_FRAC_BITS)
#define SWRZ__TILE_SIZE 64

static void swrz__pool_fill_impl(
        swrz_texture_t *fb,
        const uint32_t x,
        const uint32_t y,
        const uint32_t w,
        const uint32_t h)
{
    uint8_t *data = (uint8_t *)fb->data;
    size_t row_size = 0;

    row_size = w * sizeof(uint32_t);
    uint8_t *row =
        data + (size_t)y * fb->row_pitch + 
        (size_t)x * sizeof(uint32_t);

    for (uint32_t col = 0; col < w; ++col) {
        uint32_t pixel = 0xff0000ff;
        memcpy(row + col * sizeof(pixel), &pixel, sizeof(pixel));
    }

    for (uint32_t i = 1; i < h; i++) 
    {
        memcpy(row + (size_t)i * fb->row_pitch,
               row,
               row_size);
    }
}


typedef struct
{
    uint32_t x0;
    uint32_t y0;
    uint32_t x1;
    uint32_t y1;
} swrz__rect_t;

SWRZ__INLINE uint16_t swrz__sign_bit(
        const int64_t x) {
    return (uint16_t)(x >> (sizeof(x) * 8 - 1));
}

static uint16_t swrz__build_masks_scalar(
        const int64_t a,
        const int64_t b,
        const int64_t c)
{
    int64_t c0 = 0;
    int64_t c1 = 0;
    int64_t c2 = 0;
    int64_t c3 = 0;
    uint16_t mask = 0;

    c0 = c + 0 * b;
    c1 = c + 1 * b;
    c2 = c + 2 * b;
    c3 = c + 3 * b;

    mask |= swrz__sign_bit(c0 + 0 * a) & (1 << 0);
    mask |= swrz__sign_bit(c0 + 1 * a) & (1 << 1);
    mask |= swrz__sign_bit(c0 + 2 * a) & (1 << 2);
    mask |= swrz__sign_bit(c0 + 3 * a) & (1 << 3);
    mask |= swrz__sign_bit(c1 + 0 * a) & (1 << 4);
    mask |= swrz__sign_bit(c1 + 1 * a) & (1 << 5);
    mask |= swrz__sign_bit(c1 + 2 * a) & (1 << 6);
    mask |= swrz__sign_bit(c1 + 3 * a) & (1 << 7);
    mask |= swrz__sign_bit(c2 + 0 * a) & (1 << 8);
    mask |= swrz__sign_bit(c2 + 1 * a) & (1 << 9);
    mask |= swrz__sign_bit(c2 + 2 * a) & (1 << 10);
    mask |= swrz__sign_bit(c2 + 3 * a) & (1 << 11);
    mask |= swrz__sign_bit(c3 + 0 * a) & (1 << 12);
    mask |= swrz__sign_bit(c3 + 1 * a) & (1 << 13);
    mask |= swrz__sign_bit(c3 + 2 * a) & (1 << 14);
    mask |= swrz__sign_bit(c3 + 3 * a) & (1 << 15);

    return mask;
}


static void swrz__pool_partial(
        swrz_texture_t *fb,
        const int64_t a[3],
        const int64_t b[3],
        const int64_t c[3],
        const uint32_t x,
        const uint32_t y)
{
    uint16_t tl_mask = 0;
    uint16_t br_mask = 0;
    uint16_t full_mask = 0;
    uint16_t inside_mask = 0;

    uint32_t tile_px = 0;
    uint32_t tile_py = 0;

    int64_t tile_fx = 0;
    int64_t tile_fy = 0;

    tile_px = x * SWRZ__TILE_SIZE;
    tile_py = y * SWRZ__TILE_SIZE;

    tile_fx = (int64_t)tile_px * SWRZ__FIXED_ONE;
    tile_fy = (int64_t)tile_py * SWRZ__FIXED_ONE;

    for (int i = 0; i < 3; i++)
    {
        int64_t edge_c = 0;
        int64_t edge_a = 0;
        int64_t edge_b = 0;

        edge_a = a[i] * 16;
        edge_b = b[i] * 16;
        edge_c = c[i]
                + tile_fx * a[i]
                + tile_fy * b[i];


        tl_mask |= swrz__build_masks_scalar(
                edge_a,
                edge_b,
                edge_c);
        br_mask |= swrz__build_masks_scalar(
                edge_a,
                edge_b,
                edge_c
                + a[i] * 15
                + b[i] * 15);
    }
    printf("%04X ", tl_mask);
    if (tl_mask == 0xFFFF)
        return;

    full_mask = (uint16_t)~br_mask;
    inside_mask = (uint16_t)(~tl_mask & br_mask);
    while (full_mask != 0)
    {
        int bit_idx = 0;
        uint32_t subtile_px = 0;
        uint32_t subtile_py = 0;

        bit_idx = swrz__ffs(full_mask);

        bit_idx--;
        full_mask &= (uint16_t)~(1u << bit_idx);

        subtile_px = tile_px + (uint32_t)((bit_idx & 3) << 4);
        subtile_py = tile_py + (uint32_t)((bit_idx >> 2) << 4);

        swrz__pool_fill_impl(
                fb,
                subtile_px,
                subtile_py,
                16,
                16);
    }

    /*
     * TODO: rasterize the partial subtiles in inside_mask.
     */
    (void)inside_mask;
}



swrz_error_t swrz__pool_rasterize_triangle(
        swrz__pool_t *pool,
        swrz_texture_t *fb,
        const swrz_vertex_output_t *v0,
        const swrz_vertex_output_t *v1,
        const swrz_vertex_output_t *v2)
{
    int64_t v0_x = 0, v0_y = 0;
    int64_t v1_x = 0, v1_y = 0;
    int64_t v2_x = 0, v2_y = 0;
    int64_t area = 0;

    int64_t bb_fx0 = 0, bb_fy0 = 0;
    int64_t bb_fx1 = 0, bb_fy1 = 0;
    
    int64_t a[3] = {0}, b[3] = {0}, c[3] = {0};

    uint32_t tile_x0 = 0, tile_y0 = 0;
    uint32_t tile_x1 = 0, tile_y1 = 0;

    if(!pool || !fb || !v0 || !v1 || !v2)
        return SWRZ_ERR_PARAM;

    v0_x = (int32_t)((v0->position[0] / v0->position[3] + 1.0f) 
        * 0.5f * (float)fb->width * SWRZ__FIXED_ONE);
    v0_y = (int32_t)((1.0f - v0->position[1] / v0->position[3]) 
            * 0.5f * (float)fb->height * SWRZ__FIXED_ONE);
    v1_x = (int32_t)((v1->position[0] / v1->position[3] + 1.0f) 
            * 0.5f * (float)fb->width * SWRZ__FIXED_ONE);
    v1_y = (int32_t)((1.0f - v1->position[1] / v1->position[3]) 
            * 0.5f * (float)fb->height * SWRZ__FIXED_ONE);
    v2_x = (int32_t)((v2->position[0] / v2->position[3] + 1.0f) 
            * 0.5f * (float)fb->width * SWRZ__FIXED_ONE);
    v2_y = (int32_t)((1.0f - v2->position[1] / v2->position[3]) 
            * 0.5f * (float)fb->height * SWRZ__FIXED_ONE);

    area = (int64_t)(v1_x - v0_x) * (v2_y - v0_y) -
            (int64_t)(v1_y - v0_y) * (v2_x - v0_x);

    // This is inverted because we has inverted y-axis in screen space
    if(area >= 0)
        return SWRZ_ERR_OK;
    
    bb_fx0 = SWRZ__MIN3(v0_x, v1_x, v2_x);
    bb_fy0 = SWRZ__MIN3(v0_y, v1_y, v2_y);
    bb_fx1 = SWRZ__MAX3(v0_x, v1_x, v2_x);
    bb_fy1 = SWRZ__MAX3(v0_y, v1_y, v2_y);

    bb_fx0 = SWRZ__MAX(bb_fx0, 0);
    bb_fy0 = SWRZ__MAX(bb_fy0, 0);
    bb_fx1 = SWRZ__MIN(bb_fx1, (int64_t)fb->width * SWRZ__FIXED_ONE);
    bb_fy1 = SWRZ__MIN(bb_fy1, (int64_t)fb->height * SWRZ__FIXED_ONE);

    a[0] = (int64_t)v0_y - v1_y;
    b[0] = (int64_t)v1_x - v0_x;
    c[0] = (int64_t)v0_x * v1_y - (int64_t)v1_x * v0_y;

    a[1] = (int64_t)v1_y - v2_y;
    b[1] = (int64_t)v2_x - v1_x;
    c[1] = (int64_t)v1_x * v2_y - (int64_t)v2_x * v1_y;

    a[2] = (int64_t)v2_y - v0_y;
    b[2] = (int64_t)v0_x - v2_x;
    c[2] = (int64_t)v2_x * v0_y - (int64_t)v0_x * v2_y;

    tile_x0 = (uint32_t)((bb_fx0 >> SWRZ__FIXED_FRAC_BITS) / SWRZ__TILE_SIZE);
    tile_y0 = (uint32_t)((bb_fy0 >> SWRZ__FIXED_FRAC_BITS) / SWRZ__TILE_SIZE);
    tile_x1 = (uint32_t)((bb_fx1 >> SWRZ__FIXED_FRAC_BITS) / SWRZ__TILE_SIZE);
    tile_y1 = (uint32_t)((bb_fy1 >> SWRZ__FIXED_FRAC_BITS) / SWRZ__TILE_SIZE);

    for(uint32_t y = tile_y0; y <= tile_y1; y++)
    {
        for(uint32_t x = tile_x0; x <= tile_x1; x++)
        {
            int64_t fx0 = (int64_t)x * SWRZ__TILE_SIZE * SWRZ__FIXED_ONE; 
            int64_t fy0 = (int64_t)y * SWRZ__TILE_SIZE * SWRZ__FIXED_ONE; 
            int64_t fx1 = fx0 + (int64_t)SWRZ__TILE_SIZE * SWRZ__FIXED_ONE;
            int64_t fy1 = fy0 + (int64_t)SWRZ__TILE_SIZE * SWRZ__FIXED_ONE;

            bool outside = false, full = true;
            
            for(int i = 0; i < 3; i++)
            {
                int64_t min_x = 0, max_x = 0;
                int64_t min_y = 0, max_y = 0;

                int64_t min_value = 0, max_value = 0;
                if(a[i] >= 0)
                    min_x = fx0, max_x = fx1;
                else
                    min_x = fx1, max_x = fx0;

                if(b[i] >= 0)
                    min_y = fy0, max_y = fy1;
                else
                    min_y = fy1, max_y = fy0;

                min_value = a[i] * min_x + b[i] * min_y + c[i];
                max_value = a[i] * max_x + b[i] * max_y + c[i];

                /* inverted */
                if(min_value > 0)
                {
                    outside = true;
                    break;
                }

                /* inverted */
                if(max_value > 0)
                    full = false;
            }

            if(outside)
                continue;

            if(full)
            {
                swrz__pool_fill_impl(fb, 
                        x * SWRZ__TILE_SIZE, y * SWRZ__TILE_SIZE, 
                        SWRZ__TILE_SIZE, SWRZ__TILE_SIZE);
            } else {
                swrz__pool_partial(
                    fb,
                    a, b, c,
                    x, y);
            }
        }
    }

    return SWRZ_ERR_OK;
}

void swrz__pool_destroy(
        swrz__pool_t *pool)
{
    if(!pool)
        return;
    swrz__atomic_store(&pool->abort, 1);
    swrz__condvar_broadcast(pool->work_cv);
    for(int i = 0; i < pool->thread_count; i++)
    {
        swrz__thread_join(pool->threads[i]);
        swrz__thread_destroy(pool->threads[i]);
    }
    swrz__free(pool->threads);
    swrz__arena_destroy(&pool->arena);
    swrz__condvar_destroy(pool->work_cv);
    swrz__mutex_destroy(pool->mutex);
}
