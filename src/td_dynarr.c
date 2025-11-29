#include "td_dynarr.h"

#include <stdlib.h>
#include <string.h>

void tdp_dynarr_add(tdp_dynarr* arr, const void* item)
{
    if(!arr)
        return;
    if(arr->used >= arr->allocated)
    {
        size_t new_size = arr->allocated ? arr->allocated * 2 : 8;
        void* new_ptr = realloc(arr->ptr, arr->typesz * new_size);
        if(!new_ptr)
            return;
        arr->ptr = new_ptr;
        arr->allocated = new_size;
    }
    memcpy((td_u8*)arr->ptr + (arr->used++) * arr->typesz, item, arr->typesz);
}
