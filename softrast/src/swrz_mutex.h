#ifndef SWRZ_MUTEX_H
#define SWRZ_MUTEX_H

#include "softrast/swrz_error.h"

typedef struct swrz__mutex swrz__mutex_t;

swrz_error_t swrz__mutex_create(
        swrz__mutex_t **mtx);

swrz_error_t swrz__mutex_lock(
        swrz__mutex_t *mtx);

/*
swrz_error_t swrz__mutex_trylock(
        swrz__mutex_t *mutex);
*/

swrz_error_t swrz__mutex_unlock(
        swrz__mutex_t *mtx);

void swrz__mutex_destroy(
        swrz__mutex_t *mtx);

#endif
