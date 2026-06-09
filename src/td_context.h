#ifndef TD_CONTEXT_H
#define TD_CONTEXT_H

#include <td_def.h>
#include <td_error.h>

#include <td_renderer.h>
#include "td_debug.h"

#ifdef TD_BUILD_DEBUG
typedef struct tdp_debug_context
{
    void *file;
} tdp_debug_context_t;
#endif

typedef struct tdp_context
{
    TDP_DEBUG_DEFINE(tdp_debug_context_t debug);
    td_error_t last_error;
    td_u64 init_ts;
    td_window_t *window;
} tdp_context_t;

extern tdp_context_t *tdp_ctx;
td_error_t tdp_context_init(void);
void tdp_context_exit(void);


#endif
