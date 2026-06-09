#include <td_timer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

td_u64 td_get_performance_counter(void)
{
    LARGE_INTEGER counter = {0};
    QueryPerformanceCounter(&counter);
    return (td_u64)counter.QuadPart;
}

td_u64 td_get_performance_frequency(void)
{
    LARGE_INTEGER freq = {0};
    QueryPerformanceFrequency(&freq);
    return (td_u64)freq.QuadPart;
}

void td_delay(const td_u32 ms)
{
    Sleep(ms);
}
