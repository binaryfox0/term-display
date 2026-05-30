#include "td_dynarr.h"

#include <stdlib.h>
#include <string.h>

#define TDP_DYNARR_INITIAL 4

td_error_t tdp_dynarr_new(
        tdp_dynarr_t *arr,
        const td_u64 capacity,
        const td_u64 itemsz
)
{
    if(!arr || capacity < 0 || itemsz < 0)
        return TD_ERR_INVALID_ARG;

    arr->itemsz = itemsz;
    if(capacity == 0)
        return TD_ERR_OK;

    arr->ptr = calloc(capacity, arr->itemsz);
    if(!arr->ptr)
        return TD_ERR_OUT_OF_MEMORY;

    arr->capacity = capacity;
    return TD_ERR_OK;
}

td_error_t tdp_dynarr_add(tdp_dynarr_t* arr, const void* item)
{
    if(!arr || arr->itemsz == 0)
        return TD_ERR_INVALID_ARG;
    if(arr->size >= arr->capacity)
    {
        size_t new_capacity = 0;
        void *new_ptr = 0;

        new_capacity = arr->capacity ? arr->capacity * 2 : TDP_DYNARR_INITIAL;
        new_ptr = realloc(arr->ptr, arr->itemsz * new_capacity);
        if(!new_ptr)
            return TD_ERR_OUT_OF_MEMORY;
        arr->ptr = new_ptr;
        arr->capacity = new_capacity;
    }
    memcpy((td_u8*)arr->ptr + (arr->size++) * arr->itemsz, item, arr->itemsz);
    return TD_ERR_OK;
}

td_error_t tdp_dynarr_insert(
        tdp_dynarr_t* arr, 
        const td_u64 index,
        const void* item)
{
    if (!arr || !item)
        return TD_ERR_INVALID_ARG;

    if (index > arr->size)
        return tdp_dynarr_add(arr, item);
    
    if(arr->size >= arr->capacity)
    {
        size_t new_capacity = 0;
        void *new_ptr = 0;

        new_capacity = arr->capacity ? arr->capacity * 2 : TDP_DYNARR_INITIAL;
        new_ptr = realloc(arr->ptr, arr->itemsz * new_capacity);
        if(!new_ptr)
            return TD_ERR_OUT_OF_MEMORY;
        arr->ptr = new_ptr;
        arr->capacity = new_capacity;
    }

    memmove(
        (td_u8*)arr->ptr + (index + 1) * arr->itemsz,
        (td_u8*)arr->ptr + index * arr->itemsz,
        (arr->size - index) * arr->itemsz
    );

    memcpy(
        (td_u8*)arr->ptr + index * arr->itemsz,
        item,
        arr->itemsz
    );

    arr->size++;
    return TD_ERR_OK;
}

void tdp_dynarr_destroy(tdp_dynarr_t *arr)
{
    if(!arr)
        return;
    free(arr->ptr);
    *arr = (tdp_dynarr_t){0};
}
