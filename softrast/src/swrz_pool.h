#ifndef SWRZ_POOL_H
#define SWRZ_POOL_H

#include "softrast/swrz_error.h"
#include "swrz_atomic.h"
#include "swrz_arena.h"

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

void swrz__pool_destroy(
        swrz__pool_t *pool);

#endif
