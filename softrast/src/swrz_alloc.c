#include "softrast/swrz_alloc.h"

#include <stdlib.h>

static void *swrz__default_malloc(
        void *userdata, 
        size_t size);
static void *swrz__default_realloc(
        void *userdata, 
        void *ptr,
        size_t size);
static void swrz__default_free(
        void *userdata, 
        void *ptr);

static swrz_allocator_t swrz__allocator =
{
    .malloc = swrz__default_malloc,
    .realloc = swrz__default_realloc,
    .free = swrz__default_free,
};

static void *swrz__default_malloc(
        void *userdata, 
        size_t size)
{
    (void)userdata;
    return malloc(size);
}

static void *swrz__default_realloc(
        void *userdata, 
        void *ptr,
        size_t size)
{
    (void)userdata;
    return realloc(ptr, size);
}

static void swrz__default_free(
        void *userdata, 
        void *ptr)
{
    (void)userdata;
    free(ptr);
}

swrz_error_t swrz_set_allocator(
        const swrz_allocator_t *allocator)
{
    if(!allocator)
    {
        swrz__allocator.malloc  = swrz__default_malloc;
        swrz__allocator.realloc = swrz__default_realloc;
        swrz__allocator.free    = swrz__default_free;
        return SWRZ_ERR_OK;
    }

    if(
            !allocator->malloc ||
            !allocator->realloc ||
            !allocator->free)
        return SWRZ_ERR_PARAM;

    swrz__allocator = *allocator;
    return SWRZ_ERR_OK;
}

void *swrz__malloc(
        size_t size)
{
    return swrz__allocator.malloc(
            swrz__allocator.userdata, size);
}

void *swrz__realloc(
        void *ptr, 
        size_t size)
{
    return swrz__allocator.realloc(
            swrz__allocator.userdata, ptr, size);
}

void swrz__free(
        void *ptr)
{
    swrz__allocator.free(swrz__allocator.userdata, ptr);
}
