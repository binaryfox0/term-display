#include "swrz_arena.h"

#include <string.h>
#include "swrz_alloc_priv.h"
#include "swrz_utils.h"

// TODO: add option for fine-grained control
#define SWRZ__ARENA_REGION_CAPACITY (64 * 1024)

swrz_error_t swrz__arena_reset(
        swrz__arena_t *arena)
{
    if(!arena)
        return SWRZ_ERR_PARAM;

    for(
            swrz__arena_region_t *region = arena->begin; 
            region != NULL; region = region->next)
        region->size = 0;
    arena->cur = arena->begin;
    return SWRZ_ERR_OK;
}

static swrz__arena_region_t *swrz__arena_region_create(
        const size_t size)
{
    size_t capacity = 0;
    swrz__arena_region_t *region = NULL;
        
    capacity = 
        SWRZ__MAX(SWRZ__ARENA_REGION_CAPACITY, size);
    region = 
        swrz__malloc(sizeof(*region) + capacity);
    if(!region)
        return NULL;

    region->next = NULL;
    region->size = 0;
    region->capacity = capacity;

    return region;
}

void *swrz__arena_malloc(
        swrz__arena_t *arena,
        size_t size)
{
    void *ptr = NULL;
    if(!arena)
        return NULL;

    if(arena->cur == NULL)
    {
        arena->cur = swrz__arena_region_create(size);
        arena->begin = arena->cur;
    }

    while(
            arena->cur->size + size > arena->cur->capacity &&
                arena->cur->next)
        arena->cur = arena->cur->next;
    
    if(arena->cur->size + size > arena->cur->capacity)
    {
        arena->cur->next = swrz__arena_region_create(size);
        arena->cur = arena->cur->next;
    }

    ptr = &arena->cur->data[arena->cur->size];
    arena->cur->size += size;
    return ptr;
}

void swrz__arena_destroy(
        swrz__arena_t *arena)
{
    swrz__arena_region_t *region = NULL;
    if(!arena)
        return;

    region = arena->begin;
    while(region)
    {
        swrz__arena_region_t *current = region;
        region = region->next;
        swrz__free(current);
    }

    arena->begin = NULL;
    arena->cur = NULL;
}
