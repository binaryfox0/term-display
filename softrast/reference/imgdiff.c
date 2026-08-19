#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <aparse.h>
#include <lodepng.h>

#define debug aparse_prog_debug
#define info aparse_prog_info
#define warn aparse_prog_warn
#define error aparse_prog_error

#define WIDTH  1920
#define HEIGHT 1080

#define CHECK(x, err, label) \
    if(((err) = x) != 0) goto label

typedef struct 
{
    uint8_t *data;
} png_t;

static bool png_decode_rgba(
        const char *path,
        png_t *out)
{
    unsigned err = 0;
    LodePNGState state = {0};
    uint8_t *png = NULL;
    size_t png_size = 0;
    uint32_t width = 0, height = 0;

    if(!path || !out)
        return false;

    CHECK(lodepng_load_file(&png, &png_size, 
                path), err, cleanup);

    lodepng_state_init(&state);
    state.info_raw.bitdepth = 8;
    state.info_raw.colortype = LCT_RGBA;

    CHECK(lodepng_decode(
            &out->data, 
            &width, &height, 
            &state, 
            png, png_size), err, cleanup);

    if(width != WIDTH || height != HEIGHT)
    {
        error("expected png at \"%s\" to be %dx%d px", path, 
                WIDTH, HEIGHT);
        info("png resolution: %ux%u px", width, height);
        err = 0xFFFFFFFF;
    }

    for (unsigned i = 0; i < state.info_png.text_num; i++) 
    {
        debug(
            "%s = %s",
            state.info_png.text_keys[i],
            state.info_png.text_strings[i]
        );
    }

cleanup:
    if(err != 0 && err != 0xFFFFFFFF)
    {
        error("failed to decode png from \"%s\"", path);
        info("reason: %s", lodepng_error_text(err));
    }
    lodepng_state_cleanup(&state);
    free(png);
    return err == 0;
}

static inline void png_destroy(
        png_t *png)
{
    if(!png)
        return;
    free(png->data);
}



int main(int argc, char **argv)
{
    const char *ref_fname = NULL;
    const char *test_fname = NULL;
    const char *out_fname = "diff.png";
    aparse_arg main_args[] =
    {
        aparse_arg_string("reference", 
                &ref_fname, 0, 
                "Path to reference framebuffer image"),
        aparse_arg_string("test", 
                &test_fname, 0, 
                "Path to test framebuffer image"),
        aparse_arg_option(
                "-o", "--output", 
                &out_fname, 0, APARSE_ARG_TYPE_STRING, 
                "Path to saved final framebuffer frame (default: diff.png)"),
        aparse_arg_end_marker
    };

    int ret = 1;
    unsigned err = 0;

    png_t ref_png = {0};
    png_t test_png = {0};

    unsigned char *ref = NULL;
    unsigned char *test = NULL;
    unsigned char *diff = NULL;

    size_t pixel_count = 0;
    size_t diff_count = 0;

    if(aparse_parse(argc, argv, 
                    main_args, NULL,
                    "softrast image diff tool") != APARSE_STATUS_OK)
        return 1; 

    if(!png_decode_rgba(ref_fname, &ref_png))
        return 1;
    if(!png_decode_rgba(test_fname, &test_png))
        goto cleanup;

    ref = ref_png.data;
    test = test_png.data;

    pixel_count = (size_t)WIDTH * HEIGHT;
    diff = malloc(pixel_count * 4);
    if (!diff)
    {
        error("failed to allocate diff image\n");
        goto cleanup;
    }

    for (size_t i = 0; i < pixel_count; i++)
    {
        size_t p = i * 4;

        if (ref[p + 0] != test[p + 0] ||
            ref[p + 1] != test[p + 1] ||
            ref[p + 2] != test[p + 2] ||
            ref[p + 3] != test[p + 3])
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

    info("writing differences into \"%s\"", out_fname);
    CHECK(lodepng_encode32_file(out_fname, diff, 
                WIDTH, HEIGHT), err, cleanup);

    info("different pixels: %zu / %zu (%.2f%%)",
            diff_count, pixel_count,
            100.0 * (double)diff_count / (double)pixel_count);

    ret = 0;

cleanup:
    if(err != 0)
    {
        error("failed to encode png to \"%s\"", out_fname);
        info("reason: %s", lodepng_error_text(err));
    }

    free(diff);
    png_destroy(&ref_png); 
    png_destroy(&test_png);
    return ret;
}
