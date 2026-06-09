#include <td_misc.h>

#include <stdio.h>
#include <stdarg.h>

#include "td_context.h"
#include <td_timer.h>

#ifdef TD_BUILD_DEBUG
td_error_t td_debug_set(const char *name)
{
    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;
    if(!name)
    {
        if(!tdp_ctx->debug.file)
            return TD_ERR_OK;
        fclose(tdp_ctx->debug.file);
        tdp_ctx->debug.file = 0;
        return TD_ERR_OK;
    }

    if(tdp_ctx->debug.file)
        fclose(tdp_ctx->debug.file);
    tdp_ctx->debug.file = fopen(name, "w");
    if(!tdp_ctx->debug.file)
        return TD_ERR_IO;
    setvbuf(tdp_ctx->debug.file, 0, _IONBF, 0);
    return TD_ERR_OK;
}

td_error_t td_debug_log(const char *fmt, ...)
{
    FILE *file = 0;
    td_u64 ts = 0;
    va_list va;

    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;
    if(!tdp_ctx->debug.file)
        return TD_ERR_OK;
    file = tdp_ctx->debug.file;
    ts = td_get_ticks();
    fprintf(file, "[%08llu.%03llu]: app: ", ts / 1000, ts % 1000);
    va_start(va, fmt);
    vfprintf(file, fmt, va);
    va_end(va);
    fputc('\n', file);
    return TD_ERR_OK;
}
#endif
