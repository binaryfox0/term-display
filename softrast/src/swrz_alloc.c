#include "softrast/swrz_alloc.h"
#include "swrz_alloc_priv.h"

#include <stdlib.h>
#include <string.h>

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

void *swrz__calloc(
        size_t nmemb,
        size_t size)
{
    void *ptr = swrz__allocator.malloc(
            swrz__allocator.userdata, nmemb * size);
    if(!ptr)
        return NULL;
    memset(ptr, 0, nmemb * size);
    return ptr;
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

void *swrz__aligned_alloc(
        size_t alignment, 
        size_t size)
{
    void *raw = NULL;
    uintptr_t address = 0;
    uintptr_t aligned = 0;

    if (alignment == 0 ||
        (alignment & (alignment - 1)) != 0)
        return NULL;

    if (size > SIZE_MAX - alignment + 1 - sizeof(void *))
        return NULL;

    raw = swrz__malloc(size + alignment - 1 + sizeof(void *));
    if(!raw)
        return NULL;

    address = (uintptr_t)raw + sizeof(void *);
    aligned = (address + alignment - 1) &
              ~(uintptr_t)(alignment - 1);

    ((void **)aligned)[-1] = raw;
    return (void *)aligned;
}

void swrz__aligned_free(
        void *p)
{
    if (!p)
        return;

    swrz__free(((void **)p)[-1]);
}
