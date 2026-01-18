#include <stdlib.h>

#include "td_main.h"

#include "example_utils.h"
#include "td_texture.h"

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

static td_texture *displayed_image = 0;
static td_ivec2 imgsz = {0};

td_ivec2 ratio_new_size(const td_ivec2 old, const td_ivec2 size)
{
    if (!size.x)
        return (td_ivec2){(old.x * size.y) / old.y, size.y};
    if (!size.y)
        return (td_ivec2){size.x, (old.y * size.x) / old.x};
    return size;
}

TD_INLINE td_bool vec2_larger(td_ivec2 vec1, td_ivec2 vec2)
{
    return vec1.x > vec2.x || vec1.y > vec2.y;
}

void resize_callback(td_ivec2 new_size)
{    
    static const td_vertex_attrib attribs[] = {TDVA_POSITION_2D, TDVA_UV_COORDS };

    td_ivec2 tmp = ratio_new_size(imgsz, (td_ivec2){.x=new_size.x});
    if(vec2_larger(tmp, new_size))
        tmp = ratio_new_size(imgsz, (td_ivec2){.y=new_size.y});

    // compute corners in NDC
    td_vec2 bottom_left  = pos_to_ndc((td_ivec2){.y=tmp.y - 1}, new_size);
    td_vec2 bottom_right = pos_to_ndc((td_ivec2){.x=tmp.x - 1, .y=tmp.y - 1}, new_size);
    td_vec2 top_right    = pos_to_ndc((td_ivec2){.x=tmp.x - 1}, new_size);
    td_vec2 top_left     = pos_to_ndc((td_ivec2){0}, new_size);

    // copy positions correctly (2 floats each)
    memcpy(vertices + 0,  bottom_left.raw,  sizeof(float) * 2);  // v0
    memcpy(vertices + 4,  bottom_right.raw, sizeof(float) * 2);  // v1
    memcpy(vertices + 8,  top_right.raw,    sizeof(float) * 2);  // v2
    memcpy(vertices + 12, bottom_left.raw,  sizeof(float) * 2);  // v3
    memcpy(vertices + 16, top_right.raw,    sizeof(float) * 2);  // v4
    memcpy(vertices + 20, top_left.raw,     sizeof(float) * 2);  // v5

    td_clear_framebuffer();
    td_bind_texture(displayed_image);
    for(int i = 0; i < sizeof(vertices) / sizeof(vertices[0]) / 4; i++)
        td_add_vertex(vertices + i * 4, attribs, 2, td_true);
    td_render();
    fflush(stdout);
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
    int channel, width, height;

    stbi_set_flip_vertically_on_load(true);
    stbi_uc *tmp = stbi_load(path, &width, &height, &channel, 0);
    if (!tmp) {
        aparse_prog_error("unable to load image: \"%s\"", path);
        return;
    }

    displayed_image =
        td_texture_create(tmp, channel, (td_ivec2){.x=width, .y=height}, 1, 0);
    if (!displayed_image) {
        aparse_prog_error("unable to create texture from image.\n");
        free(tmp);
        return;
    }
    imgsz = (td_ivec2){.x=width, .y=height};

    td_clear_term();

    td_ivec2 current_size;
    td_option(td_opt_display_size, 1, &current_size);
    resize_callback(current_size);

    td_set_running_state(td_true);

    stop = false;
    double delta_time = 1.0, last_log = get_time();
    while (td_is_running() && !stop) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

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

    td_texture_destroy(displayed_image);
    displayed_image = 0;
}

int main(int argc, char **argv)
{
    char** images = 0;
    aparse_arg* main_args = 0;
    example_params p = parse_argv(argc, argv, (aparse_arg[]){
        aparse_arg_array("images", &images, 0, APARSE_ARG_TYPE_STRING, 0, "Images to be displayed")
    }, 1, &main_args);

    int images_count = main_args[0].size;
    free(main_args);

    if (td_init() == td_false || start_logging("statics.txt")) {
        return 1;
    }

    use_params(p);

    td_set_clear_color((td_rgba){0, 0, 0, 255});
    td_set_resize_callback(resize_callback);
    td_set_key_callback(key_callback);
    for(int i = 0; i < images_count && images && !force_stop; i++)
        display_image(images[i]);
    if(images)
        free(images);

    td_quit();
    stop_logging();

    return 0;
}
