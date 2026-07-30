#include "swrz_mutex.h"
#include "swrz_mutex_priv.h"

#include "swrz_alloc_priv.h"

swrz_error_t swrz__mutex_create(
        swrz__mutex_t **mtx)
{
    swrz__mutex_t *tmp = NULL;
    if(!mtx)
        return SWRZ_ERR_PARAM;

    tmp = swrz__malloc(sizeof(*tmp));
    if(!tmp)
        return SWRZ_ERR_NO_MEM;

    if(pthread_mutex_init(
                &tmp->handle,
                0) != 0)
    {
        swrz__free(tmp);
        return SWRZ_ERR_THREAD;
    }

    *mtx = tmp;
    return SWRZ_ERR_OK;
}

swrz_error_t swrz__mutex_lock(
        swrz__mutex_t *mtx)
{
    if(!mtx)
        return SWRZ_ERR_PARAM;

    if(pthread_mutex_lock(&mtx->handle) != 0)
        return SWRZ_ERR_THREAD; 
    return SWRZ_ERR_OK;
}

/*
swrz_error_t swrz__mutex_trylock(
        swrz__mutex_t *mutex)
{
    if(!mutex)
        return SWRZ_ERR_PARAM;

    if(pthread_mutex_trylock(&mutex->handle) != 0)
        return SWRZ_ERR_BUSY; 
    return SWRZ_ERR_OK;
}
*/

swrz_error_t swrz__mutex_unlock(
        swrz__mutex_t *mtx)
{
    if(!mtx)
        return SWRZ_ERR_PARAM;

    pthread_mutex_unlock(
            &mtx->handle);
    return SWRZ_ERR_OK;
}

void swrz__mutex_destroy(
        swrz__mutex_t *mtx)
{
    if(!mtx)
        return;

    pthread_mutex_destroy(
            &mtx->handle);
    swrz__free(mtx);
}

