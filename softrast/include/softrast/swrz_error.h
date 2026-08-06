#ifndef SWRZ_ERROR_H
#define SWRZ_ERROR_H

typedef enum
{
    SWRZ_ERR_OK,
    SWRZ_ERR_GENERIC,
    SWRZ_ERR_UNHANDLED,
    SWRZ_ERR_PARAM,
    SWRZ_ERR_NO_MEM,
    SWRZ_ERR_THREAD,
    SWRZ_ERR_INVALID
} swrz_error_t;

#endif
