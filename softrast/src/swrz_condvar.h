#ifndef SWRZ_CONDVAR_H
#define SWRZ_CONDVAR_H

#include <stdint.h>
#include "softrast/swrz_error.h"

typedef struct swrz__condvar swrz__condvar_t;
typedef struct swrz__mutex swrz__mutex_t;

swrz_error_t swrz__condvar_create(
        swrz__condvar_t **cv);

swrz_error_t swrz__condvar_wait(
        swrz__condvar_t *cv,
        swrz__mutex_t *mtx);

/*
swrz_error_t swrz__condvar_timedwait(
        swrz__condvar_t *cv,
        swrz__mutex_t *mutex,
        const uint32_t timeout_ms);
*/

swrz_error_t swrz__condvar_signal(
        swrz__condvar_t *cv);

swrz_error_t swrz__condvar_broadcast(
        swrz__condvar_t *cv);

void swrz__condvar_destroy(
        swrz__condvar_t *cv);

#endif
