#ifndef TDP_DYNAMIC_ARRAY
#define TDP_DYNAMIC_ARRAY

#include "td_def.h"

typedef struct tdp_dynarr {
    td_u64 allocated, used, typesz;
    void* ptr;
} tdp_dynarr;

void tdp_dynarr_add(tdp_dynarr* arr, const void* item);

#endif
