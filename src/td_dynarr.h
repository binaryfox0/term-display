#ifndef TD_DYNAMIC_ARRAY
#define TD_DYNAMIC_ARRAY

#include "td_def.h"

#define tdp_dynarr_get(arr, type, idx) (((type*)(arr)->ptr)[idx])

typedef struct tdp_dynarr {
    td_u64 allocated, used, typesz;
    void* ptr;
} tdp_dynarr;

td_bool tdp_dynarr_new(
        tdp_dynarr *arr,
        const td_u64 cap,
        const td_u64 typesz
);

td_bool tdp_dynarr_add(
        tdp_dynarr* arr, 
        const void* item
);

td_bool tdp_dynarr_insert(
        tdp_dynarr* arr, 
        const td_u64 index, 
        const void* item
);

#endif
