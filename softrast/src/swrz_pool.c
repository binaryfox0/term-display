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
    int64_t x0;
    int64_t y0;
    int64_t x1;
    int64_t y1;
} swrz__rectf_t;
    
typedef struct
{
    uint32_t x0;
    uint32_t y0;
    uint32_t x1;
    uint32_t y1;
} swrz__rect_t;

#define SWRZ__SIGN_BIT(x) ((x) >> (sizeof((x)) * 8 - 1))

SWRZ__INLINE swrz__rect_t swrz__rectf_to_rect(
        const swrz__rectf_t *rect) {
    return (swrz__rect_t){
            .x0 = (uint32_t)(rect->x0 >> SWRZ__FIXED_FRAC_BITS),
            .y0 = (uint32_t)(rect->y0 >> SWRZ__FIXED_FRAC_BITS),
            .x1 = (uint32_t)(rect->x1 >> SWRZ__FIXED_FRAC_BITS),
            .y1 = (uint32_t)(rect->y1 >> SWRZ__FIXED_FRAC_BITS)};
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

    mask |= SWRZ__SIGN_BIT(c0 + 0 * a) & (1 << 0);
    mask |= SWRZ__SIGN_BIT(c0 + 1 * a) & (1 << 1);
    mask |= SWRZ__SIGN_BIT(c0 + 2 * a) & (1 << 2);
    mask |= SWRZ__SIGN_BIT(c0 + 3 * a) & (1 << 3);
    mask |= SWRZ__SIGN_BIT(c1 + 0 * a) & (1 << 4);
    mask |= SWRZ__SIGN_BIT(c1 + 1 * a) & (1 << 5);
    mask |= SWRZ__SIGN_BIT(c1 + 2 * a) & (1 << 6);
    mask |= SWRZ__SIGN_BIT(c1 + 3 * a) & (1 << 7);
    mask |= SWRZ__SIGN_BIT(c2 + 0 * a) & (1 << 8);
    mask |= SWRZ__SIGN_BIT(c2 + 1 * a) & (1 << 9);
    mask |= SWRZ__SIGN_BIT(c2 + 2 * a) & (1 << 10);
    mask |= SWRZ__SIGN_BIT(c2 + 3 * a) & (1 << 11);
    mask |= SWRZ__SIGN_BIT(c3 + 0 * a) & (1 << 12);
    mask |= SWRZ__SIGN_BIT(c3 + 1 * a) & (1 << 13);
    mask |= SWRZ__SIGN_BIT(c3 + 2 * a) & (1 << 14);
    mask |= SWRZ__SIGN_BIT(c3 + 3 * a) & (1 << 15);

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
#define SWRZ__MAX_PLANES 3

swrz_error_t swrz__pool_rasterize_triangle(
        swrz__pool_t *pool,
        swrz_texture_t *fb,
        const swrz_vertex_output_t *v0,
        const swrz_vertex_output_t *v1,
        const swrz_vertex_output_t *v2)
{
    int64_t v0_x = 0, v1_x = 0, v2_x = 0;
    int64_t v0_y = 0, v1_y = 0, v2_y = 0;
    int64_t area = 0;

    swrz__rectf_t bboxf = {0};
    swrz__rect_t bbox = {0};
    swrz__rect_t bbox_tile = {0};

    int64_t tri_a[SWRZ__MAX_PLANES] = {0}, 
            tri_b[SWRZ__MAX_PLANES] = {0},
            tri_c[SWRZ__MAX_PLANES] = {0},
            tri_eo[SWRZ__MAX_PLANES] = {0},
            tri_ei[SWRZ__MAX_PLANES] = {0};

    int64_t a[SWRZ__MAX_PLANES] = {0},
            b[SWRZ__MAX_PLANES] = {0},
            c[SWRZ__MAX_PLANES] = {0},
            eo[SWRZ__MAX_PLANES] = {0},
            ei[SWRZ__MAX_PLANES] = {0};

    /* we inverted y position because we are mimick OpenGL */
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

    /* inverted */
    area = (int64_t)(v1_x - v0_x) * (v0_y - v2_y) -
        (int64_t)(v0_y - v1_y) * (v2_x - v0_x);
    if(area < 0)
        return SWRZ_ERR_OK;
    
    bboxf.x0 = SWRZ__MIN3(v0_x, v1_x, v2_x);
    bboxf.y0 = SWRZ__MIN3(v0_y, v1_y, v2_y);
    bboxf.x1 = SWRZ__MAX3(v0_x, v1_x, v2_x);
    bboxf.y1 = SWRZ__MAX3(v0_y, v1_y, v2_y);

    bboxf.x0 = SWRZ__MAX(bboxf.x0, 0);
    bboxf.y0 = SWRZ__MAX(bboxf.y0, 0);
    bboxf.x1 = SWRZ__MIN(bboxf.x1, (int64_t)fb->width << SWRZ__FIXED_FRAC_BITS);
    bboxf.y1 = SWRZ__MIN(bboxf.y1, (int64_t)fb->height << SWRZ__FIXED_FRAC_BITS);

    bbox = swrz__rectf_to_rect(&bboxf);
    bbox_tile.x0 = bbox.x0 >> SWRZ__TILE_ORDER;
    bbox_tile.y0 = bbox.y0 >> SWRZ__TILE_ORDER;
    bbox_tile.x1 = bbox.x1 >> SWRZ__TILE_ORDER;
    bbox_tile.y1 = bbox.y1 >> SWRZ__TILE_ORDER;

    // coefficients was invertes due to inverted y coords
tri_a[0] = v1_y - v0_y;
tri_a[1] = v2_y - v1_y;
tri_a[2] = v0_y - v2_y;

tri_b[0] = v0_x - v1_x;
tri_b[1] = v1_x - v2_x;
tri_b[2] = v2_x - v0_x;

tri_c[0] = v1_x * v0_y - v0_x * v1_y;
tri_c[1] = v2_x * v1_y - v1_x * v2_y;
tri_c[2] = v0_x * v2_y - v2_x * v0_y;

    for(int i = 0; i < SWRZ__MAX_PLANES; i++)
    {
        if(tri_a[i] > 0)
            tri_eo[i] += tri_a[i];
        else
            tri_ei[i] -= tri_a[i];

        if(tri_b[i] > 0)
            tri_eo[i] += tri_b[i];
        else
            tri_ei[i] -= tri_b[i];
    }

    for(int i = 0; i < SWRZ__MAX_PLANES; i++)
    {
        // mul by TILE_SIZE, not lshift TILE_ORDER to prevent UB
        a[i] = tri_a[i] * SWRZ__TILE_SIZE;
        b[i] = tri_b[i] * SWRZ__TILE_SIZE;
        c[i] = tri_c[i] + tri_a[i] * bboxf.x0 + tri_b[i] * bboxf.y0;
        eo[i] = tri_eo[i] * SWRZ__TILE_SIZE;
        ei[i] = tri_ei[i] * SWRZ__TILE_SIZE;
    }

    for(uint32_t tile_y = bbox_tile.y0; tile_y < bbox_tile.y1; tile_y++)
    {
        bool in = false; // inside triangle for this row
        int64_t cx[SWRZ__MAX_PLANES] = {0};
        for(int i = 0; i < SWRZ__MAX_PLANES; i++)
            cx[i] = c[i];

        for(uint32_t tile_x = bbox_tile.x0; tile_x < bbox_tile.x1; tile_x++)
        {
            int out = 0, partial = 0;
            for(int i = 0; i < SWRZ__MAX_PLANES; i++)
            {
                int64_t plane_out = 0, plane_partial = 0;
                plane_out = cx[i] + eo[i];
                plane_partial = cx[i] + ei[i] - 1;
                out |= SWRZ__SIGN_BIT(plane_out);
                partial |= SWRZ__SIGN_BIT(plane_partial) & (1 << i);
            }

            if(out)
            {
                printf("out\n");
                if(in)
                    break; // nothing left, exit...
            } else if(partial) {
                printf("partial\n");
                in = true;
            } else {
                printf("full\n");
                in = true;
                swrz__pool_fill_impl(fb, 
                        tile_x * SWRZ__TILE_SIZE,
                        tile_y * SWRZ__TILE_SIZE,
                        SWRZ__TILE_SIZE, SWRZ__TILE_SIZE,
                        0xff0000ff);
            }

            for(int i = 0; i < SWRZ__MAX_PLANES; i++)
                cx[i] += a[i];
        }

        for(int i = 0; i < SWRZ__MAX_PLANES; i++)
            c[i] += b[i];
    }

    (void)bbox_tile;
    (void)pool;
    (void)swrz__pool_fill_impl;
    (void)swrz__build_masks_scalar;
    
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
