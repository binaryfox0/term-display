#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <aparse.h>
#include <stb_image.h>
#include <stb_image_write.h>

#define debug aparse_prog_debug
#define info aparse_prog_info
#define warn aparse_prog_warn
#define error aparse_prog_error

int main(int argc, char **argv)
{
    static const int width = 1920;
    static const int height = 1080;

    const char *ref_fname = NULL;
    const char *test_fname = NULL;
    aparse_arg main_args[] =
    {
        aparse_arg_string("reference", 
                &ref_fname, 0, "reference image"),
        aparse_arg_string("test", 
                &test_fname, 0, "test image"),
        aparse_arg_end_marker
    };

    int ret = 1;
    int ref_width = 0;
    int ref_height = 0;
    int ref_channels = 0;

    int test_width = 0;
    int test_height = 0;
    int test_channels = 0;

    unsigned char *reference = NULL;
    unsigned char *test = NULL;
    unsigned char *diff = NULL;

    size_t pixel_count = 0;
    size_t diff_count = 0;

    if(aparse_parse(argc, argv, 
                    main_args, NULL,
                    "softrast image diff tool") != APARSE_STATUS_OK)
        return 1; 

    reference = stbi_load(
        argv[1],
        &ref_width,
        &ref_height,
        &ref_channels,
        4
    );
    if(!reference)
    {
        error("failed to load reference image\n");
        goto cleanup;
    }

    test = stbi_load(
        argv[2],
        &test_width,
        &test_height,
        &test_channels,
        4
    );
    if(!test)
    {
        error("failed to load test image\n");
        goto cleanup;
    }

    if (ref_width != width || ref_height != height ||
        test_width != width || test_height != height)
    {
        error("images must both be 1920x1080\n");
        goto cleanup;
    }

    pixel_count = (size_t)width * height;
    diff = malloc(pixel_count * 4);
    if (!diff)
    {
        error("failed to allocate diff image\n");
        goto cleanup;
    }

    for (size_t i = 0; i < pixel_count; i++)
    {
        size_t p = i * 4;

        if (reference[p + 0] != test[p + 0] ||
            reference[p + 1] != test[p + 1] ||
            reference[p + 2] != test[p + 2] ||
            reference[p + 3] != test[p + 3])
        {
            diff[p + 0] = 255;
            diff[p + 1] = 255;
            diff[p + 2] = 255;
            diff[p + 3] = 255;

            diff_count++;
        }
        else
        {
            diff[p + 0] = 0;
            diff[p + 1] = 0;
            diff[p + 2] = 0;
            diff[p + 3] = 255;
        }
    }

    info("writing differences into diff.png\n");
    stbi_write_png(
        "diff.png",
        width,
        height,
        4,
        diff,
        width * 4
    );

    info("different pixels: %zu / %zu (%.2f%%)\n",
            diff_count, pixel_count,
            100.0 * (double)diff_count / (double)pixel_count);

    ret = 0;

cleanup:
    free(diff);
    stbi_image_free(reference);
    stbi_image_free(test);
    return ret;
}