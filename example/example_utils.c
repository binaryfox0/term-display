#include "example_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


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

exu_paramaters_t exu_parse_args(
    int argc, char **argv,
    aparse_arg *custom_args,
    int custom_count,
    aparse_arg **merged_args
)
{
    exu_paramaters_t p = {
        .auto_resize = TD_FALSE,
        .px_w = 2,
        .px_h = 1,
//        .display_type = td_display_truecolor,
        .rotation = 0,
        .max_fps = 60
    };

    const char *pos_raw = 0;
    const char *size_raw = 0;

    const aparse_arg example_args[] = {
        aparse_arg_option(
            "-autorsz", "--auto-resize",
            &p.auto_resize, sizeof(p.auto_resize),
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
            &p.px_w, sizeof(p.px_w),
            APARSE_ARG_TYPE_UNSIGNED,
            "Pixel width of display in terminal cells"
        ),
        aparse_arg_option(
            "-pxh", "--pixel-height",
            &p.px_h, sizeof(p.px_h),
            APARSE_ARG_TYPE_UNSIGNED,
            "Pixel height of display in terminal cells"
        ),
        aparse_arg_option(
            "-type", "--display-type",
            &p.display_type, sizeof(p.display_type),
            APARSE_ARG_TYPE_UNSIGNED,
            "Type of display (grayscale, truecolor, etc.)"
        ),
        aparse_arg_option(
            "-rot", "--display-rotate",
            &p.rotation, sizeof(p.rotation),
            APARSE_ARG_TYPE_UNSIGNED,
            "Display orientation / rotation"
        ),
        aparse_arg_option(
            "-fps", "--maximum-fps",
            &p.max_fps, sizeof(p.max_fps),
            APARSE_ARG_TYPE_UNSIGNED,
            "Maximum Frame-per-Second of display"
        ),
        aparse_arg_end_marker
    };

    const int example_count =
        (int)(sizeof(example_args) / sizeof(example_args[0])) - 1;

    const int total_count = custom_count + example_count + 1;
    aparse_arg *args = calloc(total_count, sizeof(*args));
    if (!args)
        exit(EXIT_FAILURE);

    memcpy(args, custom_args, custom_count * sizeof(*args));
    memcpy(args + custom_count, example_args, example_count * sizeof(*args));

    if (aparse_parse(argc, argv, args, 0,
            "Example program of term-display library")
        == APARSE_STATUS_FAILURE)
    {
        free(args);
        exit(EXIT_FAILURE);
    }

    if (pos_raw &&
        sscanf(pos_raw, "%d,%d",
               &p.pos.x,
               &p.pos.y) != 2)
    {
        aparse_prog_error("invalid display position: \"%s\"", pos_raw);
        free(args);
        exit(EXIT_FAILURE);
    }

    if (size_raw &&
        sscanf(size_raw, "%dx%d",
               &p.size.x,
               &p.size.y) != 2)
    {
        aparse_prog_error("invalid display size: \"%s\"", size_raw);
        free(args);
        exit(EXIT_FAILURE);
    }

    if (p.max_fps == 0) {
        aparse_prog_error("invalid max fps was specified");
        aparse_prog_info("this can make this example unable to exit");
        free(args);
        exit(EXIT_FAILURE);
    }

    if (merged_args)
        *merged_args = args;
    else
        free(args);

    return p;
}
