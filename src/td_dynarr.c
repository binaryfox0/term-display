#include "td_dynarr.h"

#include <stdlib.h>
#include <string.h>

td_bool tdp_dynarr_new(
        tdp_dynarr *arr,
        const td_u64 cap,
        const td_u64 typesz
)
{
    if(!arr || !typesz)
        return td_false;

    arr->typesz = typesz;
    if(!cap)
        return td_true;

    arr->ptr = calloc(cap, arr->typesz);
    if(!arr->ptr)
    {
        arr->typesz = 0;
        return td_false;
    }

    arr->allocated = cap;

    return td_true;
}

td_bool tdp_dynarr_add(tdp_dynarr* arr, const void* item)
{
    if(!arr || arr->typesz == 0)
        return td_false;
    if(arr->used >= arr->allocated)
    {
        size_t new_size = arr->allocated ? arr->allocated * 2 : 8;
        void* new_ptr = realloc(arr->ptr, arr->typesz * new_size);
        if(!new_ptr)
            return td_false;
        arr->ptr = new_ptr;
        arr->allocated = new_size;
    }
    memcpy((td_u8*)arr->ptr + (arr->used++) * arr->typesz, item, arr->typesz);
    return td_true;
}

td_bool tdp_dynarr_insert(
        tdp_dynarr* arr, 
        const td_u64 index,
        const void* item)
{
    if (!arr || !item || arr->typesz == 0)
        return td_false;

    if (index > arr->used)
        return tdp_dynarr_add(arr, item);

    if (arr->used >= arr->allocated)
    {
        size_t new_size = arr->allocated ? arr->allocated * 2 : 8;
        void* new_ptr = realloc(arr->ptr, arr->typesz * new_size);
        if (!new_ptr)
            return td_false;
        arr->ptr = new_ptr;
        arr->allocated = new_size;
    }

    memmove(
        (td_u8*)arr->ptr + (index + 1) * arr->typesz,
        (td_u8*)arr->ptr + index * arr->typesz,
        (arr->used - index) * arr->typesz
    );

    memcpy(
        (td_u8*)arr->ptr + index * arr->typesz,
        item,
        arr->typesz
    );

    arr->used++;
    return td_true;
}
