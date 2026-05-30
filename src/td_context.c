#include "td_context.h"

#include <stdlib.h>

// extern
tdp_context_t *tdp_ctx = 0;

td_error_t tdp_context_init(void)
{
    if(tdp_ctx)
        return TD_ERR_OK;
    tdp_ctx = calloc(1, sizeof(*tdp_ctx));
    if(!tdp_ctx)
        return TD_ERR_OUT_OF_MEMORY;
    return TD_ERR_OK;
}

void tdp_context_exit(void)
{
    free(tdp_ctx);
    tdp_ctx = 0;
}
