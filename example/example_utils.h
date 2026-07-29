#ifndef EXAMPLE_UTILS_H
#define EXAMPLE_UTILS_H

#include <td_def.h>
#include <aparse.h>

#define error aparse_prog_error
#define warn aparse_prog_warn
#define info aparse_prog_info

#define ARRSZ(arr) (sizeof((arr)) / sizeof((arr)[0]))

typedef struct
{
    int auto_resize;
    td_ivec2 pos;
    td_ivec2 size;
    td_u8 px_w, px_h;
    int display_type;
    td_u8 rotation;
    td_u32 max_fps;
} exu_paramaters_t;

typedef void exu_file_t;

int exu_ask_yes_no(void);

int exu_parse_args(
    const int argc, char** argv,
    aparse_arg* custom_args,
    exu_paramaters_t *out
);

exu_file_t *exu_fopen(void);
void exu_fprintf(
        exu_file_t *file, 
        const td_u64 log_interval,
        td_u64 *last_log, 
        const char *fmt, ...);
void exu_fclose(exu_file_t *file);

#endif
