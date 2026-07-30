#include "swrz_condvar.h"
#include "swrz_mutex_priv.h"

#include "swrz_alloc_priv.h"

struct swrz__condvar
{
    CONDITION_VARIABLE handle;
};

swrz_error_t swrz__condvar_create(
        swrz__condvar_t **cv)
{
    swrz__condvar_t *tmp = NULL;
    if(!cv)
        return SWRZ_ERR_PARAM;
    tmp = swrz__malloc(sizeof(*tmp));
    if(!tmp)
        return SWRZ_ERR_NO_MEM;

    InitializeConditionVariable(
            &cond->handle);
    return SWRZ_ERR_OK;
}

swrz_error_t swrz__condvar_wait(
        swrz__condvar_t *cv,
        swrz__mutex_t *mtx)
{
    if(!cv || !mtx)
        return SWRZ_ERR_PARAM;

    SleepConditionVariableCS(
            &cv->handle,
            &mtx->handle,
            INFINITE);
    return SWRZ_ERR_OK;
}

/*
swrz_error_t swrz__condvar_timedwait(
        swrz__condvar_t *cond,
        swrz__mutex_t *mutex,
        uint32_t timeout_ms)
{
    if(!cond || !mutex)
        return SWRZ_ERR_PARAM;

    if(SleepConditionVariableCS(
                &cond->handle,
                &mutex->handle,
                timeout_ms))
        return SWRZ_ERR_OK;

    if(GetLastError() == ERROR_TIMEOUT)
        return SWRZ_ERR_TIMEOUT;

    return SWRZ_ERR_GENERIC;
}
*/

swrz_error_t swrz__condvar_signal(
        swrz__condvar_t *cv)
{
    if(!cv)
        return SWRZ_ERR_PARAM;

    WakeConditionVariable(
            &cv->handle);
    return SWRZ_ERR_OK;
}

swrz_error_t swrz__condvar_broadcast(
        swrz__condvar_t *cv)
{
    if(!cv)
        return SWRZ_ERR_PARAM;

    WakeAllConditionVariable(
            &cv->handle);
    return SWRZ_ERR_OK;
}

void swrz__condvar_destroy(
        swrz__condvar_t *cv)
{
    if(!cv)
        return;

    swrz__free(cv);
}
