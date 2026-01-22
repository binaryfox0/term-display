#ifndef TD_DEBUG
#define TD_DEBUG

#include "td_black_magic.h"

#ifdef TD_BUILD_DEBUG
extern void* __tdp_debug_file_handle__;

int tdp_debug_init(void);

#define tdp_debug_quit() fclose(__tdp_debug_file_handle__);
#define tdp_debug_log(fmt, ...) fprintf(__tdp_debug_file_handle__, "%s: " fmt "\n", __func__ __VA_ARGS__)

#   define tdp_debug(func, ...) __td_cat(tdp_debug_, func)(__VA_ARGS__)
#else
#   define tdp_debug(...) (0)
#endif

#endif
