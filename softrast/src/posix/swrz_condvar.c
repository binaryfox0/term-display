#include "swrz_condvar.h"

#include <errno.h>
#include "swrz_mutex_priv.h"
#include "swrz_alloc_priv.h"

struct swrz__condvar
{
    pthread_cond_t handle;
};

swrz_error_t swrz__condvar_create(
        swrz__condvar_t **cv)
{
    int res = 0;
    swrz__condvar_t *tmp = NULL;
    int attr_init = 0;
    pthread_condattr_t attr;

    if(!cv)
        return SWRZ_ERR_PARAM;

    tmp = swrz__malloc(sizeof(*tmp));
    if (!tmp)
        return SWRZ_ERR_NO_MEM;

    res = pthread_condattr_init(&attr);
    if (res != 0)
        goto cleanup;
    attr_init = 1;

    res = pthread_condattr_setclock(
            &attr, CLOCK_MONOTONIC);
    if (res != 0)
        goto cleanup;

    res = pthread_cond_init(&tmp->handle, &attr);

cleanup:
    if(attr_init)
        pthread_condattr_destroy(&attr);

    if (res != 0)
    {
        swrz__free(tmp);
        return SWRZ_ERR_THREAD;
    }

    *cv = tmp;
    return SWRZ_ERR_OK;
}

swrz_error_t swrz__condvar_wait(
        swrz__condvar_t *cv,
        swrz__mutex_t *mtx)
{
    int res = 0;
    if(!cv || !mtx)
        return SWRZ_ERR_PARAM;

    res = pthread_cond_wait(
            &cv->handle,
            &mtx->handle);
    return res != 0 ? SWRZ_ERR_THREAD : SWRZ_ERR_OK;
}

/*
swrz_error_t swrz__condvar_timedwait(
        swrz__condvar_t *cond,
        swrz__mutex_t *mutex,
        const uint32_t timeout_ms)
{
    int ret = 0;
    struct timespec ts = {0};
    if(!cond || !mutex)
        return SWRZ_ERR_PARAM;

    if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return SWRZ_ERR_GENERIC;

    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec +=
        (long)(timeout_ms % 1000) * 1000000L;

    if(ts.tv_nsec >= 1000000000L)
    {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }

    ret = pthread_cond_timedwait(
            &cond->handle,
            &mutex->handle,
            &ts);

    if(ret == 0)
        return SWRZ_ERR_OK;

    if(ret == ETIMEDOUT)
        return SWRZ_ERR_TIMEOUT;

    return SWRZ_ERR_GENERIC;
}
*/
swrz_error_t swrz__condvar_signal(
        swrz__condvar_t *cv)
{
    if(!cv)
        return SWRZ_ERR_PARAM;

    pthread_cond_signal(
            &cv->handle);
    return SWRZ_ERR_OK;
}

swrz_error_t swrz__condvar_broadcast(
        swrz__condvar_t *cv)
{
    if(!cv)
        return SWRZ_ERR_PARAM;

    pthread_cond_broadcast(
            &cv->handle);
    return SWRZ_ERR_OK;
}

void swrz__condvar_destroy(
        swrz__condvar_t *cv)
{
    if(!cv)
        return;
    pthread_cond_destroy(&cv->handle);
    swrz__free(cv);
}

