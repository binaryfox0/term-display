#ifndef SWRZ_MUTEX_PRIV_H
#define SWRZ_MUTEX_PRIV_H

#include <pthread.h>

struct swrz__mutex
{
    pthread_mutex_t handle;
};

#endif
