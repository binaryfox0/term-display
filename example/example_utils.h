#ifndef EXAMPLE_UTILITIES_H
#define EXAMPLE_UTILITIES_H

#include <td_def.h>
#include <aparse.h>

#define error aparse_prog_error
#define warn aparse_prog_warn
#define info aparse_prog_info

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


int exu_ask_yes_no(void);

exu_paramaters_t exu_parse_args(
    const int argc, char** argv,
    aparse_arg* custom_args, int args_count, aparse_arg** merged_args
);


#endif
