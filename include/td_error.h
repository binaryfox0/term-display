#ifndef TD_ERROR_H
#define TD_ERROR_H

typedef enum td_error
{
    TD_ERR_OK = 0,

    TD_ERR_GENERIC,
    TD_ERR_IO,
    TD_ERR_NOT_TERMINAL,
    TD_ERR_NOT_INITIALIZED,
    TD_ERR_OUT_OF_MEMORY,
    TD_ERR_INVALID_ARG,
    TD_ERR_FORBIDDEN
} td_error_t;

#endif
