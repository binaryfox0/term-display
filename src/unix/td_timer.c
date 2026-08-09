#include <td_timer.h>

#include <errno.h>
#include <time.h>

td_u64 td_get_performance_counter(void)
{
    struct timespec ts = {0};

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ((td_u64)ts.tv_sec * 1000000000ULL)
         + (td_u64)ts.tv_nsec;
}

td_u64 td_get_performance_frequency(void)
{
    return 1000000000ULL;
}

void td_delay(const td_u32 ms)
{
    struct timespec req = {0};

    req.tv_sec  = ms / 1000;
    req.tv_nsec = (long)(ms % 1000) * 1000000L;

    while(nanosleep(&req, &req) == -1)
    {
        if(errno != EINTR)
            break;
    }
}
