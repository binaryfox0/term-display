#include "example_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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
    return to_string("%d:%d.%06d", (term_u32) time / 60, (term_u32) time % 60,
                     (term_u32) (time * 1000000) % 1000000);
}

char *optarg = NULL;
int optind = 1;
int opterr = 1;
int optopt = '?';

static int nextchar = 0;

int getopt_long(int argc, char * const argv[], const char *optstring,
                const option *longopts, int *longindex) {
    optarg = NULL;

    if (optind >= argc)
        return -1;

    const char *arg = argv[optind];

    if (arg[0] != '-' || arg[1] == '\0')
        return -1;

    // Handle "--"
    if (strcmp(arg, "--") == 0) {
        optind++;
        return -1;
    }

    // Handle long option
    if (arg[1] == '-') {
        const char *name = arg + 2;
        const char *val = strchr(name, '=');
        size_t namelen = val ? (size_t)(val - name) : strlen(name);

        for (int i = 0; longopts[i].name; i++) {
            if (strncmp(name, longopts[i].name, namelen) == 0 &&
                strlen(longopts[i].name) == namelen) {
                if (longindex)
                    *longindex = i;
                if (longopts[i].has_arg == REQUIRED_ARG) {
                    if (val) {
                        optarg = (char *)(val + 1);
                    } else if (optind + 1 < argc) {
                        optarg = argv[++optind];
                    } else {
                        if (opterr)
                            fprintf(stderr, "option '--%s' requires an argument\n", name);
                        optind++;
                        return '?';
                    }
                } else if (longopts[i].has_arg == OPTIONAL_ARG) {
                    optarg = val ? (char *)(val + 1) : NULL;
                }
                optind++;
                if (longopts[i].flag) {
                    *(longopts[i].flag) = longopts[i].val;
                    return 0;
                }
                return longopts[i].val;
            }
        }

        if (opterr)
            fprintf(stderr, "unrecognized option '%s'\n", arg);
        optind++;
        return '?';
    }

    // Handle short options like -a -b -c
    char opt = arg[++nextchar];
    const char *optdecl = strchr(optstring, opt);
    if (!optdecl) {
        if (opterr)
            fprintf(stderr, "illegal option -- %c\n", opt);
        optopt = opt;
        if (arg[++nextchar] == '\0') {
            optind++;
            nextchar = 0;
        }
        return '?';
    }

    if (optdecl[1] == ':') {
        // Argument required
        if (arg[nextchar + 1] != '\0') {
            optarg = (char *)(arg + nextchar + 1);
            optind++;
            nextchar = 0;
        } else if (optind + 1 < argc) {
            optarg = argv[++optind];
            optind++;
            nextchar = 0;
        } else {
            if (opterr)
                fprintf(stderr, "option requires an argument -- %c\n", opt);
            optopt = opt;
            optind++;
            nextchar = 0;
            return '?';
        }
    } else {
        // No argument
        if (arg[nextchar + 1] == '\0') {
            optind++;
            nextchar = 0;
        }
    }

    return opt;
}

int example_tdopt(int argc, char* const argv[])
{
    const option longopts[] = {
        { "auto-resize",    NO_ARG,       0, 'a' },
        { "display-size",   REQUIRED_ARG, 0, 's' },
        { "pixel-width",    REQUIRED_ARG, 0, 'w' },
        { "pixel-height",   REQUIRED_ARG, 0, 'h' },
        { "display-type",   REQUIRED_ARG, 0, 't' },
        { "display-rotate", REQUIRED_ARG, 0, 'r' },
        { 0, 0, 0, 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "aw:h:t:r:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'a':
                term_u8 enable = 1;
                td_option(td_opt_auto_resize, 0, &enable);
                break;
            case 's':
                term_ivec2 size;
                sscanf(optarg, "%dx%d", &size.x, &size.y);
                td_option(td_opt_display_size, 0, &size);
                break;
            // case 'w':
            //     pixel_width = atoi(optarg);
            //     break;
            // case 'h':
            //     pixel_height = atoi(optarg);
            //     break;
            // case 't':
            //     display_type = optarg;
            //     break;
            case 'r':
                term_u8 tmp = atoi(optarg);
                td_option(td_opt_display_rotate, 0, &tmp);
                break;
            case '?':
            default:
                return '?';
        }
    }
    return 0;
}