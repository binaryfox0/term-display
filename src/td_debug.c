#include "td_debug.h"

#include <stdio.h>
#include "td_context.h"

void tdp_debug_exit(void)
{
#ifdef TD_BUILD_DEBUG
    if(tdp_ctx->debug.file)
        fclose(tdp_ctx->debug.file);
#endif
}
