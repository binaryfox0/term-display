#include <td_timer.h>

#include "td_context.h"

td_u64 td_get_ticks(void)
{
    td_u64 delta = 0, freq = 0;
    delta = td_get_performance_counter() - 
        (tdp_ctx ? tdp_ctx->init_ts : 0);
    freq = td_get_performance_frequency();
    return 
        (delta / freq) * 1000 +
        ((delta % freq) * 1000) / freq;
}

td_u64 td_get_ticks_ns(void)
{
    td_u64 delta = 0, freq = 0;
    delta = td_get_performance_counter() - 
        (tdp_ctx ? tdp_ctx->init_ts : 0);
    freq = td_get_performance_frequency();
    return 
        (delta / freq) * 1000000000ULL +
        ((delta % freq) * 1000000000ULL) / freq;
}
