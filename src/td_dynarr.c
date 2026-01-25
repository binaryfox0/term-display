#include "td_dynarr.h"

#include <stdlib.h>
#include <string.h>

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
