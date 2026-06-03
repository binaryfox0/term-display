#ifndef EXAMPLE_UTILITIES_H
#define EXAMPLE_UTILITIES_H

#include <td_def.h>
#include <aparse.h>

#define error aparse_prog_error
#define warn aparse_prog_warn
#define info aparse_prog_info

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
#define ARRSZ(arr) (sizeof((arr)) / sizeof((arr)[0]))

typedef struct exu_paramaters 
{
    td_bool auto_resize;
    td_ivec2 pos;
    td_ivec2 size;
    td_u8 px_w, px_h;
    int display_type;
    td_u8 rotation;
    td_u32 max_fps;
} exu_paramaters_t;

double get_time();
char *to_string(const char *format, ...);
char *to_timestamp(double time);

int exu_ask_yes_no(void);

exu_paramaters_t exu_parse_args(
    const int argc, char** argv,
    aparse_arg* custom_args, int args_count, aparse_arg** merged_args
);

void use_params(const exu_paramaters_t *p);

#endif
