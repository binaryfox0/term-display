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

static void swrz__pool_fill(
    swrz_texture_t *fb,
    const int32_t x,
    const int32_t y)
{
    uint8_t *data = (uint8_t *)fb->data;
    const uint32_t row_size =
        SWRZ__TILE_SIZE * sizeof(uint32_t);
    uint32_t x0 = (uint32_t)x * SWRZ__TILE_SIZE;
    uint32_t y0 = (uint32_t)y * SWRZ__TILE_SIZE;

    uint8_t *row =
        data + (size_t)y0 * fb->row_pitch +
        (size_t)x0 * sizeof(uint32_t);

    for (uint32_t col = 0; col < SWRZ__TILE_SIZE; ++col) {
        uint32_t pixel = 0xff0000ff;
        memcpy(row + col * sizeof(pixel), &pixel, sizeof(pixel));
    }

    for (uint32_t i = 1; i < SWRZ__TILE_SIZE; ++i) {
        memcpy(row + (size_t)i * fb->row_pitch,
               row,
               row_size);
    }
}


swrz_error_t swrz__pool_rasterize_triangle(
        swrz__pool_t *pool,
        swrz_texture_t *fb,
        const swrz_vertex_output_t *v0,
        const swrz_vertex_output_t *v1,
        const swrz_vertex_output_t *v2)
{
    int32_t v0_x = 0, v0_y = 0;
    int32_t v1_x = 0, v1_y = 0;
    int32_t v2_x = 0, v2_y = 0;
    int64_t area = 0;

    int32_t bb_x0 = 0, bb_y0 = 0;
    int32_t bb_x1 = 0, bb_y1 = 0;

    int64_t a[3] = {0}, b[3] = {0}, c[3] = {0};

    int32_t tile_x0 = 0, tile_y0 = 0;
    int32_t tile_x1 = 0, tile_y1 = 0;


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
    
    
    bb_x0 = SWRZ__MIN3(v0_x, v1_x, v2_x);
    bb_y0 = SWRZ__MIN3(v0_y, v1_y, v2_y);
    bb_x1 = SWRZ__MAX3(v0_x, v1_x, v2_x);
    bb_y1 = SWRZ__MAX3(v0_y, v1_y, v2_y);

    a[0] = (int64_t)v0_y - v1_y;
    b[0] = (int64_t)v1_x - v0_x;
    c[0] = (int64_t)v0_x * v1_y - (int64_t)v1_x * v0_y;

    a[1] = (int64_t)v1_y - v2_y;
    b[1] = (int64_t)v2_x - v1_x;
    c[1] = (int64_t)v1_x * v2_y - (int64_t)v2_x * v1_y;

    a[2] = (int64_t)v2_y - v0_y;
    b[2] = (int64_t)v0_x - v2_x;
    c[2] = (int64_t)v2_x * v0_y - (int64_t)v0_x * v2_y;

    tile_x0 = (bb_x0 >> SWRZ__FIXED_FRAC_BITS) / SWRZ__TILE_SIZE;
    tile_y0 = (bb_y0 >> SWRZ__FIXED_FRAC_BITS) / SWRZ__TILE_SIZE;
    tile_x1 = (bb_x1 >> SWRZ__FIXED_FRAC_BITS) / SWRZ__TILE_SIZE;
    tile_y1 = (bb_y1 >> SWRZ__FIXED_FRAC_BITS) / SWRZ__TILE_SIZE;

    for(int32_t y = tile_y0; y <= tile_y1; y++)
    {
        for(int32_t x = tile_x0; x <= tile_x1; x++)
        {
            int64_t x0 = (int64_t)x * SWRZ__TILE_SIZE * SWRZ__FIXED_ONE; 
            int64_t y0 = (int64_t)y * SWRZ__TILE_SIZE * SWRZ__FIXED_ONE; 
            int64_t x1 = x0 + (int64_t)SWRZ__TILE_SIZE * SWRZ__FIXED_ONE;
            int64_t y1 = y0 + (int64_t)SWRZ__TILE_SIZE * SWRZ__FIXED_ONE;

            bool outside = false, full = true;
            
            for(int i = 0; i < 3; i++)
            {
                int64_t min_x = 0, max_x = 0;
                int64_t min_y = 0, max_y = 0;

                int64_t min_value = 0, max_value = 0;
                if(a[i] >= 0)
                    min_x = x0, max_x = x1;
                else
                    min_x = x1, max_x = x0;

                if(b[i] >= 0)
                    min_y = y0, max_y = y1;
                else
                    min_y = y1, max_y = y0;

                min_value = a[i] * min_x + b[i] * min_y + c[i];
                max_value = a[i] * max_x + b[i] * max_y + c[i];

                /* inverted */
                if(min_value > 0)
                {
                    outside = true;
                    break;
                }

                if(max_value > 0)
                    full = false;
            }

            if(outside)
                continue;

            if(full)
            {
                swrz__pool_fill(fb, x, y);
                printf("full tile: (%d, %d)\n", x, y);
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
