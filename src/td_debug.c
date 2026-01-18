#include "td_debug.h"

#include <stdio.h>

void* __tdp_debug_file_handle__ = 0;

int tdp_debug_init(void)
{
    __tdp_debug_file_handle__ = fopen("td_log.txt", "w");
    if(!__tdp_debug_file_handle__)
        return 1;
    if (setvbuf(__tdp_debug_file_handle__, 0, _IONBF, 0) != 0) {
        fclose(__tdp_debug_file_handle__);
        return 1;
    }
    return 0;
}

