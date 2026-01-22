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
    double speed = 0.01;
    example_params p = parse_argv(argc, argv, 
        (aparse_arg[1]){
            aparse_arg_option("--speed", 0, &speed, sizeof(speed), APARSE_ARG_TYPE_FLOAT, "The speed of RGB scrolling effect")
        }, 1, 0
    );

    double hz = speed / (2.0 * M_PI);
    if(hz > 3.0)
    {
        aparse_prog_warn("speed=%.2f produces %.3f Hz RGB scrolling, which may trigger photosensitive epilepsy", speed, hz);
        aparse_prog_info("recommended maximum speed=~%.2f", 3.0 * 2.0 * M_PI);
        for(;;)
        {
            printf("%s: " APARSE_ANSIES("\x1b[1;34m") "info" APARSE_ANSIES("\x1b[0m") ": do you want to continue (y/n): ", __aparse_progname);
            int ch = getchar();
            while(getchar() != '\n') {}
            if(ch == 'y')
                break;
            else if(ch == 'n')
                return 0;
            aparse_prog_info("invalid answer");
        }
    }
 
    if (td_init() == td_false || start_logging("statics.txt"))
        return 1;
 
    use_params(&p);

    td_vertex_attrib attribs[] = { TDVA_POSITION_2D, TDVA_UV_COORDS};
    td_font* font = td_default_font((td_rgba){255, 255, 255, 255}, (td_rgba){0});

    td_ivec2 size = { 0 };
    double elapsed = 0.0;
    double delta_time = 0.0, last_log = get_time();
    const double max_dt = 1.0 / p.max_fps;
    while (td_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        td_set_clear_color(calculate_rgb(elapsed));

        char *string = to_string("%f", fps);
        td_texture *texture =
            td_render_string(font, string, strlen(string), &size);

        td_bind_texture(texture);
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
            td_add_vertex(vertices + i * 4, attribs, sizeof(attribs) / sizeof(attribs[0]), td_true);
        td_texture_destroy(texture);

        td_render();
        elapsed += delta_time * speed;

        while ((delta_time = get_time() - start_frame) < max_dt) {}
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();
        }
        free(string);
    }
    td_destroy_font(font);
    td_quit();
    stop_logging();
    return 0;
}
