#include "swrz_mutex.h"

#include "swrz_alloc_priv.h"
#include "swrz_mutex_priv.h"

swrz_error_t swrz__mutex_create(
        swrz__mutex_t **mtx)
{
    swrz__mutex_t *tmp = NULL;
    if(!mtx)
        return SWRZ_ERR_NO_MEM;

    tmp = swrz__malloc(sizeof(*tmp));
    if(!tmp)
        return SWRZ_ERR_NO_MEM;

    InitializeCriticalSection(
            &tmp->handle);
    *mtx = tmp;
    return SWRZ_ERR_OK;
}

swrz_error_t swrz__mutex_lock(
        swrz__mutex_t *mtx)
{
    if(!mtx)
        return SWRZ_ERR_PARAM;

    EnterCriticalSection(&mtx->handle);
    return SWRZ_ERR_OK;
}

/*
swrz_error_t swrz__mutex_trylock(
        swrz__mutex_t *mutex)
{
    if(!mutex)
        return SWRZ_ERR_PARAM;

    if(TryEnterCriticalSection(&mutex->handle) == 0)
        return SWRZ_ERR_BUSY;
    return SWRZ_ERR_OK;
}
*/


swrz_error_t swrz__mutex_unlock(
        swrz__mutex_t *mtx)
{
    if(!mtx)
        return SWRZ_ERR_PARAM;

    LeaveCriticalSection(
            &mtx->handle);
    return SWRZ_ERR_OK;
}

void swrz__mutex_destroy(
        swrz__mutex_t *mtx)
{
    if(!mtx)
        return;

    DeleteCriticalSection(
            &mtx->handle);
    swrz__free(mtx);
}

