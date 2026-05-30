#ifndef TD_RENDERER_PRIV_H
#define TD_RENDERER_PRIV_H

#include <td_error.h>
#include <td_def.h>

td_error_t tdp_renderer_init(void);
void tdp_renderer_exit(void);
td_error_t tdp_renderer_resize(const td_ivec2 logical_size);

#endif

