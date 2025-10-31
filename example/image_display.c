#include "td_main.h"

#include "example_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static float vertices[] = {
    // x, y, u, v
    0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
    0.0f, 0.0f, 1.0f, 1.0f, // bottom-right
    0.0f, 0.0f, 1.0f, 0.0f, // top-right
    0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
    0.0f, 0.0f, 1.0f, 0.0f, // top-right
    0.0f, 0.0f, 0.0f, 0.0f  // top-left
};

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

int width = 0, height = 1;
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

    // compute corners in NDC
    td_vec2 bottom_left  = pos_to_ndc(td_ivec2_init(0, tmp.y - 1), new_size);
    td_vec2 bottom_right = pos_to_ndc(td_ivec2_init(tmp.x - 1, tmp.y - 1), new_size);
    td_vec2 top_right    = pos_to_ndc(td_ivec2_init(tmp.x - 1, 0), new_size);
    td_vec2 top_left     = pos_to_ndc(td_ivec2_init(0, 0), new_size);

    // copy positions correctly (2 floats each)
    memcpy(vertices + 0,  bottom_left.raw,  sizeof(float) * 2);  // v0
    memcpy(vertices + 4,  bottom_right.raw, sizeof(float) * 2);  // v1
    memcpy(vertices + 8,  top_right.raw,    sizeof(float) * 2);  // v2
    memcpy(vertices + 12, bottom_left.raw,  sizeof(float) * 2);  // v3
    memcpy(vertices + 16, top_right.raw,    sizeof(float) * 2);  // v4
    memcpy(vertices + 20, top_left.raw,     sizeof(float) * 2);  // v5
}

td_bool stop = td_false;
td_bool force_stop = td_false;

void key_callback(int key, int mods, td_key_state_t state){
    if(key == td_key_x && state == td_key_press) {
        if(mods == td_key_ctrl)
            force_stop = true;
        stop = td_true;
    }
}

char* program_name = 0;
void display_image(const char* path)
{
    int channel;

    stbi_set_flip_vertically_on_load(true);
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

    td_ivec2 current_size;
    td_option(td_opt_display_size, 1, &current_size);

    double ratio1 = (double)current_size.x / current_size.y;
    double ratio2 = (double)width / height;
    if(ratio1 > ratio2)
        width = 0;
    else 
        height = 0;

    resize_callback(current_size);
    td_set_resize_callback(resize_callback);
    td_set_key_callback(key_callback);

    td_set_running_state(td_true);
    stop = false;
    double delta_time = 1.0, last_log = get_time();
    tdr_vertex_attrib attribs[] = {TDRVA_POSITION_2D, TDRVA_UV_COORDS };
    while (td_is_running() && !stop) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();
        tdr_clear_framebuffer();
        tdr_bind_texture(displayed_image);
        for(int i = 0; i < sizeof(vertices) / sizeof(vertices[0]) / 4; i++)
            tdr_add_vertex(vertices + i * 4, attribs, 2, td_true);
        tdr_render();

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
    char** images = 0;
    aparse_arg* main_args = parse_argv(argc, argv, (aparse_arg[]){
        aparse_arg_array("images", &images, 0, APARSE_ARG_TYPE_STRING, 0, "Images to be displayed")
    }, 1);

    int images_count = main_args[0].size;
    free(main_args);

    if (td_init() || start_logging("statics.txt")) {
        return 1;
    }

    tdr_set_clear_color(td_rgba_init(0, 0, 0, 255));
    for(int i = 0; i < images_count && images && !force_stop; i++)
        display_image(images[i]);
    if(images)
        free(images);

    td_free();
    stop_logging();

    return 0;
}
