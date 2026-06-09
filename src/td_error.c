#include "td_error.h"

#include "td_utils.h"

const char *td_strerror(const td_error_t err)
{
    static const char *errmsg[__TD_ERR_MAX__] =
    {
        [TD_ERR_OK]                 = "no error",
        [TD_ERR_GENERIC]            = "generic error",
        [TD_ERR_IO]                 = "input/output error",
        [TD_ERR_NOT_TERMINAL]       = "not a terminal",
        [TD_ERR_NOT_INITIALIZED]    = "not initialized",
        [TD_ERR_OUT_OF_MEMORY]      = "out of memory",
        [TD_ERR_INVALID_ARG]        = "invalid argument",
        [TD_ERR_EXISTED]            = "already existed"
    };

    if(TDP_OUT_RANGE(err, 0, __TD_ERR_MAX__ - 1))
        return 0;

    return errmsg[err];
}

