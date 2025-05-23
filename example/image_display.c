#include "td_main.h"
#include "td_font.h"

#include "example_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

char *get_program_name(char *in)
{
#ifdef TD_PLATFORM_UNIX
#include <libgen.h>
    return basename(in);
#elif TD_PLATFORM_WINDOWS
    static char out[_MAX_FNAME] = { 0 };
    _splitpath_s(in, 0, 0, 0, 0, out, _MAX_FNAME, 0, 0);
    _splitpath_s(in, 0, 0, 0, 0, 0, 0, &out[strlen(out)], _MAX_EXT);
    return out;
#else
    return in;
#endif
}

int width, height;
td_texture *original_image = 0, *displayed_image = 0;

TD_INLINE td_bool is_landscape(td_ivec2 size)
{
    return size.x > size.y;
}

td_ivec2 ratio_new_size(const td_ivec2 old, const td_ivec2 size)
{
    if (!size.x)
        return td_ivec2_init((old.x * size.y) / old.y, size.y);
    if (!size.y)
        return td_ivec2_init(size.x, (old.y * size.x) / old.x);
    return size;
}

TD_INLINE td_bool vec2_larger(td_ivec2 vec1, td_ivec2 vec2)
{
    return vec1.x > vec2.x || vec1.y > vec2.y;
}

void resize_callback(td_ivec2 new_size)
{
    if (width)
        width = new_size.x;
    if (height)
        height = new_size.y;

    td_ivec2 tmp =
        ratio_new_size(tdt_get_size(original_image),
                       td_ivec2_init(width, height));
    if (vec2_larger(tmp, new_size)) {
        if (width) {
            height = new_size.y;
            width = 0;
        }
        else if (height) {
            width = new_size.x;
            height = 0;
        }
    }

    if (displayed_image)
        tdt_free(displayed_image);
    displayed_image = tdt_copy(original_image);
    tdt_resize(displayed_image, td_ivec2_init(width, height));
}

td_bool stop = td_false;
void key_callback(int key, int mods, td_key_state_t state){
    if(key == td_key_escape && state == td_key_press)
        stop = td_true;
}

char* program_name = 0;
void display_image(const char* path)
{
    int channel;

    stbi_uc *tmp = stbi_load(path, &width, &height, &channel, 0);
    if (!tmp) {
        printf("Error: %s: Unable to load image file.\n", program_name);
        return;
    }

    original_image =
        tdt_create(tmp, channel, td_ivec2_init(width, height), 1, 0);
    if (!original_image) {
        printf("Error: %s: Unable to load image file.\n", program_name);
        free(tmp);
        return;
    }
    width = 0;

    td_ivec2 current_size;
    td_option(td_opt_display_size, 1, &current_size);
    resize_callback(current_size);
    td_set_resize_callback(resize_callback);

    td_set_running_state(td_true);
    double delta_time = 1.0, last_log = get_time();
    while (td_is_running() && !stop) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();
        td_set_color(rgba_init(0, 0, 0, 255));
        td_copy_texture(displayed_image, td_vec2_init(-1.f, 1.f),
                             TEXTURE_MERGE_CROP);
        td_show();

        delta_time = get_time() - start_frame;
        if (start_frame - last_log >= LOG_INTERVAL) {
            char *string = to_string("%f", fps);
            if (string) {
                write_log("FPS: %s", string);
                last_log = get_time();
                free(string);
            }
        }
    }

    tdt_free(original_image);
    original_image = 0;
    tdt_free(displayed_image);
    displayed_image = 0;
}

int main(int argc, char **argv)
{
    program_name = get_program_name(argv[0]);
    if(argc < 2) {
        printf("Usage: %s <image>\n", program_name);
        return 1;
    }

    if (td_init() || start_logging("statics.txt")) {
        return 1;
    }

    display_image(argv[1]);

    td_free();
    stop_logging();

    return 0;
}
