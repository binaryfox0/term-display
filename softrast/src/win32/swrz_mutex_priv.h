#ifndef SWRZ_MUTEX_PRIV_H
#define SWRZ_MUTEX_PRIV_H

#include <windows.h>

struct swrz__mutex
{
    CRITICAL_SECTION handle;
};

#endif
