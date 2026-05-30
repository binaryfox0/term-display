#include "td_utils.h"

#include <stdio.h>
#include <string.h>

void tdp_fill_buffer(
        void* dest, 
        const void* src, 
        const td_u64 destsz, 
        const td_u64 srcsz)
{
    td_u8 *ptr = 0;
    size_t filled = 0;
    if(!destsz || !srcsz || !dest || !src) 
        return;
    if (destsz < srcsz) 
    {
        memcpy(dest, src, destsz);
        return;
    }

    ptr = (td_u8*)dest;
    memcpy(ptr, src, srcsz);
    filled = srcsz;
    ptr += srcsz;

    while (filled * 2 <= destsz) {
        memcpy(ptr, dest, filled);
        ptr += filled;
        filled *= 2;
    }

    memcpy(ptr, dest, destsz - filled);
}
