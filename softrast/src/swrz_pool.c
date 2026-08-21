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

/* Convention: < 0 is inside, > 0 is outside */

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
#define SWRZ__TILE_ORDER 6
#define SWRZ__TILE_SIZE (1 << SWRZ__TILE_ORDER)

static void swrz__pool_fill_impl(
        swrz_texture_t *fb,
        const uint32_t x,
        const uint32_t y,
        const uint32_t w,
        const uint32_t h,
        const uint32_t color)
{
    uint8_t *data = (uint8_t *)fb->data;
    size_t row_size = 0;

    row_size = w * sizeof(uint32_t);
    uint8_t *row =
        data + (size_t)y * fb->row_pitch + 
        (size_t)x * sizeof(uint32_t);

    for (uint32_t col = 0; col < w; ++col) {
        memcpy(row + col * sizeof(color), &color, sizeof(color));
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

    /* inverted */
    return (uint16_t)~mask;
}

/*
static void swrz__pool_partial(
        swrz_texture_t *fb,
        const int64_t a[3],
        const int64_t b[3],
        const int64_t c[3],
        const uint32_t x,
        const uint32_t y)
{
}
*/


swrz_error_t swrz__pool_rasterize_triangle(
        swrz__pool_t *pool,
        swrz_texture_t *fb,
        const swrz_vertex_output_t *v0,
        const swrz_vertex_output_t *v1,
        const swrz_vertex_output_t *v2)
{
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
