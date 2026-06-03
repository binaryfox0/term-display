#include <stdlib.h>

#include <td_main.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "example_utils.h"

static float vertices_uv[] = {
    0.0f, 1.0f, // bottom-left
    1.0f, 1.0f, // bottom-right
    1.0f, 0.0f, // top-right
    0.0f, 1.0f, // bottom-left
    1.0f, 0.0f, // top-right
    0.0f, 0.0f  // top-left
};

static const int indices[] = {
    2, 3, 1,
    2, 1, 0
};

static td_texture_t *displayed_image = 0;
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
    static const td_vtx_attb attribs[] = {TDVA_POSITION_2D, TDVA_UV_COORDS };

    td_ivec2 tmp = ratio_new_size(imgsz, (td_ivec2){.x=new_size.x});
    if(vec2_larger(tmp, new_size))
        tmp = ratio_new_size(imgsz, (td_ivec2){.y=new_size.y});

    td_ivec2 vertices_int[4] = {
        { .x = (new_size.x - tmp.x) / 2, .y = (new_size.y - tmp.y) / 2 }, // tl
        { .x = vertices_int[0].x + tmp.x, .y = vertices_int[0].y }, // tr
        { .x = vertices_int[0].x, .y = vertices_int[0].y + tmp.y }, // bl
        { .x = vertices_int[0].x + tmp.x, .y = vertices_int[0].y + tmp.y } // br
    };
    
    td_renderer_clear();
    td_renderer_bind_texture(displayed_image);

    float new_vertices[4 * 2] = {0};
    for(int i = 0; i < ARRSZ(indices); i++)
    {
        int vidx = indices[i];
        td_add_vertex(
                pos_to_ndc(vertices_int[vidx], new_size).raw,
                (td_vtx_attb[1]){TDVA_POSITION_2D},
                1, TD_FALSE
        );
        td_add_vertex(
                &vertices_uv[vidx * 2],
                (td_vtx_attb[1]){TDVA_UV_COORDS},
                1, TD_TRUE
        );
    }

    td_render();
    fflush(stdout);
}

td_bool stop = TD_FALSE;
td_bool force_stop = TD_FALSE;

void key_callback(td_key_token_t key, td_key_action_t action, td_key_mod_t mods)
{
    if(key == td_key_x && action == TD_KEY_PRESS)
    {
        if(mods == TD_MOD_CTRL)
            force_stop = true;
        stop = TD_TRUE;
    }
}

static char* program_name = 0;
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

    td_window_clear();

    td_ivec2 current_size;
    td_option(td_opt_display_size, 1, &current_size);
    resize_callback(current_size);

    td_window_is_running(TD_TRUE);

    stop = false;
    double delta_time = 1.0, last_log = get_time();
    while (td_window_is_running() && !stop) {
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
    for(int i = 0; i < images_count; i++)
        aparse_prog_info("[%d]: \"%s\"", i, images[i]);
    free(main_args);

    if (td_init() == TD_FALSE || start_logging("statics.txt")) {
        return 1;
    }

    use_params(&p);

    td_renderer_set_clear_color((td_rgba){0, 0, 0, 255});
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
