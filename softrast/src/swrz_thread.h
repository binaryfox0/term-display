#ifndef SWRZ_THREAD_H
#define SWRZ_THREAD_H

#include "softrast/swrz_error.h"

typedef struct swrz__thread swrz__thread_t;
typedef void (*swrz__thread_callback_t)(void *userdata);

swrz_error_t swrz__thread_create(
        swrz__thread_t **thread,
        swrz__thread_callback_t cb,
        void *userdata);

swrz_error_t swrz__thread_join(
        swrz__thread_t *thread);

void swrz__thread_destroy(
        swrz__thread_t *thread);

#endif
