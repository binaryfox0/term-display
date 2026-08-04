#ifndef SWRZ_ALLOC_H
#define SWRZ_ALLOC_H

#include <stddef.h>
#include <softrast/swrz_error.h>

typedef struct
{
    void *(*malloc)(void *userdata, size_t size);
    void *(*realloc)(void *userdata, void *ptr, size_t size);
    void (*free)(void *userdata, void *ptr);
    void *userdata;
} swrz_allocator_t;

swrz_error_t swrz_set_allocator(
        const swrz_allocator_t *allocator);
#endif
