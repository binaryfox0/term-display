#ifndef TD_DEBUG
#define TD_DEBUG

#include <td_error.h>
#include <td_timer.h>

void tdp_debug_exit(void);

#ifdef TD_BUILD_DEBUG
#   define TDP_DEBUG_LOG(fmt, ...) \
        do {                                                \
            if(tdp_ctx->debug.file) {                       \
                td_u64 ts = td_get_ticks();                 \
                fprintf(tdp_ctx->debug.file,                \
                        "[%08llu.%.03llu]: " fmt "\n",      \
                        ts / 1000, ts % 1000 __VA_OPT__(,)  \
                        __VA_ARGS__                         \
                );                                          \
            }                                               \
        } while(0)
#   define TDP_DEBUG_DEFINE(x) x
#else
#   define TDP_DEBUG_LOG(fmt, ...)
#   define TDP_DEBUG_DEFINE(x)
#endif

#endif
