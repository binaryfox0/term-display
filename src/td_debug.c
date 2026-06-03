#include "td_debug.h"

#include <stdio.h>
#include "td_context.h"

#define TDP_DEBUG_FILE "td_debug_log.txt"

td_error_t tdp_debug_init(void)
{
    TDP_DEBUG_DEFINE(tdp_debug_context_t *debug = 0);
#ifdef TD_BUILD_DEBUG
    debug = &tdp_ctx->debug;
    debug->file = fopen(TDP_DEBUG_FILE, "w");
    if(!debug->file)
        return TD_ERR_IO; 
    // if it failed, it would not be a big a problem anyway
    setvbuf(debug->file, 0, _IONBF, 0);
#endif
    return TD_ERR_OK;
}

void tdp_debug_exit(void)
{
    TDP_DEBUG_DEFINE(fclose(tdp_ctx->debug.file));
}
