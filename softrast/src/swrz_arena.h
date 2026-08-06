#ifndef SWRZ_ARENA_H
#define SWRZ_ARENA_H

#include <stdint.h>
#include <stddef.h>

#include "softrast/swrz_error.h"

typedef struct swrz__arena_region
{
    struct swrz__arena_region *next;
    size_t size;
    size_t capacity;
    uint8_t data[];
} swrz__arena_region_t;

typedef struct
{
    swrz__arena_region_t *begin;
    swrz__arena_region_t *cur;
} swrz__arena_t;

swrz_error_t swrz__arena_reset(
        swrz__arena_t *arena);

void *swrz__arena_malloc(
        swrz__arena_t *arena,
        size_t size);

void swrz__arena_destroy(
        swrz__arena_t *arena);

#endif
