#ifndef SWRZ_POOL_H
#define SWRZ_POOL_H

#include "softrast/swrz_error.h"
#include "softrast/swrz_vertex.h"
#include "swrz_atomic.h"
#include "swrz_arena.h"

struct swrz_texture;
typedef struct
{
    struct swrz__thread **threads;
    int thread_count;

    struct swrz__mutex *mutex;
    struct swrz__condvar *work_cv;
    swrz__atomic_int_t abort;
    swrz__arena_t arena;
} swrz__pool_t;

swrz_error_t swrz__pool_init(
        swrz__pool_t *pool);

swrz_error_t swrz__pool_rasterize_triangle(
        swrz__pool_t *pool,
        struct swrz_texture *fb,
        const swrz_vertex_output_t *v0,
        const swrz_vertex_output_t *v1,
        const swrz_vertex_output_t *v2);

void swrz__pool_destroy(
        swrz__pool_t *pool);

#endif
