#include "example_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
double get_time()
{
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double) counter.QuadPart / frequency.QuadPart;
}
#else
#include <time.h>
double get_time()
{
#ifdef CLOCK_MONOTONIC
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + (double) ts.tv_nsec / 1e9;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (double) tv.tv_usec / 1e6;
#endif
}
#endif

#ifdef TESTS_LOGGING
double program_start = 0;
FILE *file = 0;
term_bool start_logging(const char *filename)
{
    if (!(file = fopen(filename, "w")))
        return 1;
    if (setvbuf(file, 0, _IONBF, 0) != 0) {
        fclose(file);
        return 1;
    }
    program_start = get_time();
    return 0;
}

char *to_string_v(const char *format, va_list args);
void write_log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char *str = to_string_v(format, args);
    if (!str)
        return;
    va_end(args);
    char *timestamp = to_timestamp(get_time() - program_start);
    if (!timestamp) {
        free(str);
        return;
    }
    fprintf(file, "[%s]: %s\n", timestamp, str);
    free(timestamp);
    free(str);
}

term_bool stop_logging()
{
    return fclose(file) == EOF;
}
#endif

char *to_string_v(const char *format, va_list args)
{
    va_list copy;
    va_copy(copy, args);
    int len = vsnprintf(0, 0, format, args);
    if (len <= 0)
        return 0;
    char *out = 0;
    if (!(out = (char *) malloc(len + 1)))
        return 0;
    vsnprintf(out, len + 1, format, copy);
    return out;
}

char *to_string(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char *out = to_string_v(format, args);
    va_end(args);
    return out;
}

char *to_timestamp(double time)
{
    // Format: ...mm:ss.msx6
    return to_string("%d:%d.%06d", (term_u32) time / 60, (term_u32) time % 60,
                     (term_u32) (time * 1000000) % 1000000);
}
