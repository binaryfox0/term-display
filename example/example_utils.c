#include "example_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <td_timer.h>

int exu_ask_yes_no(void)
{
    for (;;)
    {
        int ch = 0;
        int c = 0;
        printf("%s: " __aparse_info_label
               ": do you want to continue (y/n): ",
               __aparse_progname);

        ch = getchar();

        if (ch == EOF)
        {
            info("EOF received, default to no");
            return 0;
        }

        while ((c = getchar()) != '\n' && c != EOF) {}

        if (ch == 'y')
            return 1;
        else if (ch == 'n')
            return 0;

        error("invalid answer");
    }
    return 0;
}

int exu_parse_args(
    int argc, char **argv,
    aparse_arg *extended_args,
    exu_paramaters_t *out)
{
    int ret = 0;
    exu_paramaters_t params = {
        .auto_resize = 0,
        .px_w = 2,
        .px_h = 1,
//        .display_type = td_display_truecolor,
        .rotation = 0,
        .max_fps = 60
    };

    const char *pos_raw = 0;
    const char *size_raw = 0;

    aparse_arg default_args[] = 
    {
        aparse_arg_option(
            "-autorsz", "--auto-resize",
            &params.auto_resize, sizeof(params.auto_resize),
            APARSE_ARG_TYPE_BOOL,
            "Automatic resizing the display"
        ),
        aparse_arg_option(
            "-pos", "--display-pos",
            &pos_raw, 0,
            APARSE_ARG_TYPE_STRING,
            "The position of display (x,y)"
        ),
        aparse_arg_option(
            "-sz", "--display-size",
            &size_raw, 0,
            APARSE_ARG_TYPE_STRING,
            "The size of display (wxh)"
        ),
        aparse_arg_option(
            "-pxw", "--pixel-width",
            &params.px_w, sizeof(params.px_w),
            APARSE_ARG_TYPE_UNSIGNED,
            "Pixel width of display in terminal cells"
        ),
        aparse_arg_option(
            "-pxh", "--pixel-height",
            &params.px_h, sizeof(params.px_h),
            APARSE_ARG_TYPE_UNSIGNED,
            "Pixel height of display in terminal cells"
        ),
        aparse_arg_option(
            "-type", "--display-type",
            &params.display_type, sizeof(params.display_type),
            APARSE_ARG_TYPE_UNSIGNED,
            "Type of display (grayscale, truecolor, etc.)"
        ),
        aparse_arg_option(
            "-rot", "--display-rotate",
            &params.rotation, sizeof(params.rotation),
            APARSE_ARG_TYPE_UNSIGNED,
            "Display orientation / rotation"
        ),
        aparse_arg_option(
            "-fps", "--maximum-fps",
            &params.max_fps, sizeof(params.max_fps),
            APARSE_ARG_TYPE_UNSIGNED,
            "Maximum Frame-per-Second of display"
        ),
        aparse_arg_end_marker
    };

    static const int default_count =
        (int)(sizeof(default_args) / sizeof(default_args[0])) - 1;

    aparse_arg *args = NULL;

    if(extended_args)
    {
        int extended_count = 0;
        for(aparse_arg *arg = extended_args; aparse_arg_nend(arg); arg++)
            extended_count++;
        args = malloc(
                (default_count + extended_count + 1) * sizeof(*args));
        if(!args)
            return 0;
        memcpy(args, default_args, default_count * sizeof(*default_args));
        memcpy(args + default_count, extended_args, 
                extended_count * sizeof(*extended_args));
        args[default_count + extended_count] = aparse_arg_end_marker;
    } else {
        args = default_args;
    }

    if (aparse_parse(
                argc, argv, 
                args, NULL,
            "Example program of term-display library"
        ) != APARSE_STATUS_OK)
        goto cleanup;

    if (pos_raw &&
        sscanf(pos_raw, "%d,%d",
               &params.pos.x,
               &params.pos.y) != 2)
    {
        aparse_prog_error("invalid display position: \"%s\"", pos_raw);
        goto cleanup;
    }

    if (size_raw &&
        sscanf(size_raw, "%dx%d",
               &params.size.x,
               &params.size.y) != 2)
    {
        aparse_prog_error("invalid display size: \"%s\"", size_raw);
        goto cleanup;
    }

    if (params.max_fps == 0) {
        aparse_prog_error("invalid fps was specified: %d fps", params.max_fps);
        goto cleanup;
    }

    *out = params;
    ret = 1;

cleanup:
    if(args != default_args)
        free(args);
    return ret;
}

exu_file_t *exu_fopen(void)
{
    return fopen("exu_statics.txt", "w");
}

void exu_fprintf(
        exu_file_t *file, 
        const td_u64 log_interval,
        td_u64 *last_log, 
        const char *fmt, ...)
{
    va_list va;
    td_u64 timestamp = 0ULL;
    if(!file || !fmt)
        return;

    timestamp = td_get_ticks();
    if(timestamp - *last_log < log_interval)
        return;
    *last_log = timestamp;

    fprintf(file, "[%02llu:%02llu.%03llu] ",
            timestamp / (60ULL * 1000ULL),
            (timestamp / 1000ULL) % 60ULL,
            timestamp % 1000ULL);

    va_start(va, fmt);
    vfprintf(file, fmt, va);
    va_end(va);

    fputc('\n', file);
}

void exu_fclose(
        exu_file_t *file)
{
    if(!file)
        return;
    fclose(file);
}
