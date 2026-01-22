#ifndef EXAMPLE_UTILITIES_H
#define EXAMPLE_UTILITIES_H

#include "td_def.h"
#include "aparse.h"

#define EXAMPLE_LOGGING

#ifdef EXAMPLE_LOGGING
td_bool start_logging(const char *filename);
void write_log(const char *format, ...);
td_bool stop_logging();
#else
#define start_logging(tmp) 0
#define stop_logging() 0
#define write_log(tmp1, ...)
#endif

#define LOG_INTERVAL 0.1 // In seconds

typedef struct example_params {
    td_bool auto_resize;
    td_ivec2 display_pos;
    td_ivec2 display_size;
    td_u8 px_w, px_h;
    int display_type;
    td_u8 display_orientation;
    td_u32 max_fps;
} example_params;

double get_time();
char *to_string(const char *format, ...);
char *to_timestamp(double time);

example_params parse_argv(
    const int argc, char** argv,
    aparse_arg* custom_args, int args_count, aparse_arg** merged_args
);

void use_params(const example_params *p);

#endif
