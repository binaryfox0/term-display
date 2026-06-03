#ifndef TD_DEBUG
#define TD_DEBUG

#include <td_error.h>
#include "td_black_magic.h"

td_error_t tdp_debug_init(void);
void tdp_debug_exit(void);

#ifdef TD_BUILD_DEBUG
#define tdp_debug_log(fmt, ...) fprintf(__tdp_debug_file_handle__, "%s: " fmt "\n", __func__ __VA_ARGS__)

#   define TDP_DEBUG_FUNC(func, ...) \
        __td_cat(tdp_debug_, func)(__VA_ARGS__)
#   define TDP_DEBUG_DEFINE(x) x
#else
#   define TDP_DEBUG_FUNC(...) (0)
#   define TDP_DEBUG_DEFINE(x)
#endif

#endif
