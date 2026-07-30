#include "swrz_thread.h"

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include <pthread.h>
#include "swrz_alloc_priv.h"

struct swrz__thread
{
    pthread_t handle;
    swrz__thread_callback_t cb;
    void *userdata;
    int joinable;
};

static void *swrz__thread_entry(
        void *param)
{
    swrz__thread_t *thread = param;
    thread->cb(thread->userdata);
    return 0;
}

swrz_error_t swrz__thread_create(
        swrz__thread_t **thread,
        swrz__thread_callback_t cb,
        void *userdata)
{
    int ret = 0;
    swrz__thread_t *tmp = 0;
    if(!tmp || !cb)
        return SWRZ_ERR_PARAM;

    tmp = swrz__malloc(sizeof(*tmp));
    if(!tmp)
        return SWRZ_ERR_NO_MEM;

    tmp->cb = cb;
    tmp->userdata = userdata;
    ret =
        pthread_create(
            &tmp->handle,
            NULL,
            swrz__thread_entry,
            tmp);

    if (ret != 0)
    {
        swrz__free(tmp);
        return SWRZ_ERR_THREAD;
    }

    tmp->joinable = 1;
    *thread = tmp;
    return SWRZ_ERR_OK;
}

swrz_error_t swrz__thread_join(
    swrz__thread_t *thread)
{
    int ret = 0;
    if(!thread)
        return SWRZ_ERR_PARAM;
    if(!thread->joinable)
        return SWRZ_ERR_OK;

    ret = pthread_join(
        thread->handle,
        0);
    if(ret != 0)
        return SWRZ_ERR_THREAD;

    thread->joinable = 0;
    return SWRZ_ERR_OK;
}

void swrz__thread_destroy(
        swrz__thread_t *thread)
{
    if(!thread)
        return;
    swrz__free(thread);
}
