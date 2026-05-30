#ifndef TD_DYNARR_H
#define TD_DYNARR_H

#include <td_def.h>
#include <td_error.h>

#define tdp_dynarr_get(arr, type, idx) (((type*)(arr)->ptr)[idx])

typedef struct tdp_dynarr 
{
    td_u64 capacity;
    td_u64 size;
    td_u64 itemsz;
    void *ptr;
} tdp_dynarr_t;

td_error_t tdp_dynarr_new(
        tdp_dynarr_t *arr,
        const td_u64 capacity,
        const td_u64 itemsz
);

td_error_t tdp_dynarr_add(
        tdp_dynarr_t* arr, 
        const void* item
);

td_error_t tdp_dynarr_insert(
        tdp_dynarr_t* arr, 
        const td_u64 index, 
        const void* item
);

void tdp_dynarr_destroy(tdp_dynarr_t *arr);

#endif
