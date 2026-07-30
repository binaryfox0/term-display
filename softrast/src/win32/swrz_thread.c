#include "swrz_thread.h"

#include <stdlib.h>
#include <windows.h>
#include <process.h>

#include "swrz_alloc_priv.h"

struct swrz__thread
{
    HANDLE handle;
    swrz__thread_callback_t cb;
    void *userdata;
    int joinable;
};

static DWORD WINAPI swrz__thread_entry(
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
    swrz__thread_t *tmp = 0;
    if(!thread || !cb)
        return SWRZ_ERR_PARAM;

    tmp = swrz__malloc(sizeof(*tmp));
    if(!tmp)
        return SWRZ_ERR_NO_MEM;

    tmp->cb = cb;
    tmp->userdata = userdata;
        
    tmp->handle = (HANDLE)_beginthreadex(
            NULL,
            0,
            swrz__thread_entry,
            tmp,
            0,
            NULL);

    if(!tmp->handle)
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
    DWORD res = 0;
    if(!thread)
        return SWRZ_ERR_PARAM;
    if(!thread->joinable)
        return SWRZ_ERR_OK;

    res = WaitForSingleObject(
        thread,
        INFINITE);
    if(res == WAIT_FAILED)
        return SWRZ_ERR_THREAD;
    thread->joinable = 0;
    return SWRZ_ERR_OK;
}

void swrz__thread_destroy(
        swrz__thread_t *thread)
{
    if(!thread)
        return;
    if(!thread->handle)
        CloseHandle(thread->handle);
    swrz__free(thread);
}
