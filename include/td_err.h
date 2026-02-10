#ifndef TD_ERROR_H
#define TD_ERROR_H

typedef enum td_err
{
    TD_ERR_OK = 0,
    TD_ERR_NOT_TERMINAL,
    TD_ERR_OUT_OF_MEMORY,
    TD_ERR_NOT_INITIALIZED,
    TD_ERR_INVALID_ARG,
    TD_ERR_FORBIDDEN
} td_err;

#endif
