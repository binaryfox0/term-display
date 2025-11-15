#include <math.h>               // sin
#include <string.h>             // strlen
#include <stdlib.h>             // free

#include "td_main.h"
#include "td_font.h"

#include "example_utils.h"

#define M_PI 3.14159265358979323846

static td_f32 vertices[] = {
//    x     y     u     v
    -1.0f, 1.0f, 0.0f, 1.0f, // Top-left
     0.0f, 1.0f, 1.0f, 1.0f,
     0.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
     0.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f
};

TD_INLINE td_rgba calculate_rgb(double d)
{
    return (td_rgba){
        (td_u8) ((sin(d) + 1) * 127.5),
        (td_u8) ((sin(d + (2 * M_PI / 3)) + 1) * 127.5),
        (td_u8) ((sin(d + (4 * M_PI / 3)) + 1) * 127.5),
        255
    };
}

int main(int argc, char** argv)
{
    example_params p = parse_argv(argc, argv, 0, 0, 0);
    if (td_init() || start_logging("statics.txt"))
        return 1; 
    
    use_params(p);

    td_ivec2 size = { 0 };
    double speed = 0.01, elapsed = 0.0;
    double delta_time = 0.0, last_log = get_time();
    const double max_dt = 1.0 / p.max_fps;
    tdr_vertex_attrib attribs[] = { TDRVA_POSITION_2D, TDRVA_UV_COORDS};
    while (td_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        tdr_set_clear_color(calculate_rgb(elapsed));

        char *string = to_string("%f", fps);
        td_texture *texture =
            tdf_string_texture(string, strlen(string), &size,
                                   (td_rgba){255, 255, 255, 255},
                                   (td_rgba){0, 0, 0, 0});

        tdr_bind_texture(texture);
        td_ivec2 display_sz = {0};
        td_option(td_opt_display_size, td_true, &display_sz);
        td_vec2 right_pos = pos_to_ndc((td_ivec2){.x=size.x} , display_sz);
        td_vec2 bottom_pos = pos_to_ndc((td_ivec2){.y=size.y}, display_sz);
        vertices[1 * 4 + 0] = right_pos.x;
        vertices[2 * 4 + 0] = right_pos.x;
        vertices[2 * 4 + 1] = bottom_pos.y;
        vertices[4 * 4 + 0] = right_pos.x;
        vertices[4 * 4 + 1] = bottom_pos.y;
        vertices[5 * 4 + 1] = bottom_pos.y;
        for(int i = 0; i < sizeof(vertices) / sizeof(float) / 4; i++)
            tdr_add_vertex(vertices + i * 4, attribs, sizeof(attribs) / sizeof(attribs[0]), td_true);
        tdt_free(texture);

        tdr_render();
        elapsed += speed;

        while ((delta_time = get_time() - start_frame) < max_dt) {}
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();
        }
        free(string);
    }
    td_free();
    stop_logging();
    return 0;
}
