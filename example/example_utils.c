#include "example_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "aparse.h"
#include "td_main.h"

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

#ifdef EXAMPLE_LOGGING
double program_start = 0;
FILE *file = 0;
td_bool start_logging(const char *filename)
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

td_bool stop_logging()
{
    return fclose(file) == EOF;
}
#endif

char *to_string_v(const char *format, va_list args)
{
    va_list copy;
    va_copy(copy, args);
    int len = vsnprintf(0, 0, format, copy);
    va_end(copy);
    if (len <= 0)
        return 0;
    char *out = (char *) malloc(len + 1);
    if (!out)
        return 0;

    va_copy(copy, args);
    vsnprintf(out, len + 1, format, copy);
    va_end(copy);
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
    return to_string("%d:%d.%06d", (td_u32) time / 60, (td_u32) time % 60,
                     (td_u32) (time * 1000000) % 1000000);
}


example_params parse_argv(
    const int argc, char** argv, 
    aparse_arg* custom_args, int args_count, aparse_arg** merged_args
)
{
    example_params p = {
        td_false,
        2, 1,
        td_display_truecolor,
        0,
        60
    };
    const aparse_arg example_args[] = {
        aparse_arg_option(0, "--auto-resize", &p.auto_resize, sizeof(p.auto_resize), APARSE_ARG_TYPE_BOOL, "Automatic resizing the display"),
        aparse_arg_option(0, "--pixel-width", &p.px_w, sizeof(p.px_w), APARSE_ARG_TYPE_UNSIGNED, "Pixel width of display in terminal cells"),
        aparse_arg_option(0, "--pixel-height", &p.px_h, sizeof(p.px_h), APARSE_ARG_TYPE_UNSIGNED, "Pixel height of display in terminal cells"),
        aparse_arg_option(0, "--display-type", &p.display_type, sizeof(p.display_type), APARSE_ARG_TYPE_UNSIGNED, "Type of display (grayscale, truecolor, etc."),
        aparse_arg_option(0, "--display-rotate", &p.display_orientation, sizeof(p.display_orientation), APARSE_ARG_TYPE_UNSIGNED, "Type of display (grayscale, truecolor, etc."),
        aparse_arg_option(0, "--maximum-fps", &p.max_fps, sizeof(p.max_fps), APARSE_ARG_TYPE_UNSIGNED, "Maximum Frame-per-Second of display"),
        aparse_arg_end_marker
    };
    int example_args_size = (sizeof(example_args) / sizeof(example_args[0])); // Exclude the end marker
    int _args_count = example_args_size + args_count + 1;
    aparse_arg* main_args = malloc(_args_count* sizeof(aparse_arg));
    memcpy(main_args, custom_args, args_count * sizeof(aparse_arg));
    memcpy(main_args + args_count, example_args, example_args_size * sizeof(aparse_arg));
    
    if(aparse_parse(argc, argv, main_args, "Example program of term-display library") == APARSE_STATUS_FAILURE) {
        free(main_args);
        exit(EXIT_FAILURE);
    }

    if(p.max_fps == 0) {
        aparse_prog_error("invalid max fps was specified");
        aparse_prog_info("this can make this example unable to exit");
        free(main_args);
        exit(EXIT_FAILURE);
    }

    if(!merged_args)
        free(main_args);
    else
        *merged_args = main_args;

    return p;
}

void use_params(example_params p)
{
    td_option(td_opt_auto_resize, td_false, &p.auto_resize);
    td_option(td_opt_pixel_width, td_false, &p.px_w);
    td_option(td_opt_pixel_height, td_false, &p.px_h);
    td_option(td_opt_display_type, td_false, &p.display_type);
    td_option(td_opt_display_rotate, 0, &p.display_orientation);
}
