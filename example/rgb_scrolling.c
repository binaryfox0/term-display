#include <stdio.h>
#include <math.h>               // sin

#include <td_main.h>
// #include <td_font.h>

#include "example_utils.h"

#define M_PI 3.14159265358979323846
int main(int argc, char** argv)
{
    double speed = 0.01, hz = 0.0;
    aparse_arg custom_args[] = {
           aparse_arg_option("--speed", 0, &speed,
                    sizeof(speed), APARSE_ARG_TYPE_FLOAT, 
                    "The speed of RGB scrolling effect")
    };
    exu_paramaters_t param = {0};

    td_window_t *window = 0;
    td_renderer_t *renderer = 0;
    td_u64 target_dt = 0.0, dt = 0.0;
    double elapsed = 0.0;
    td_u64 last_log = 0.0;

    param = exu_parse_args(argc, argv, 
        custom_args, 1, 0
    );
    hz = speed / (2.0 * M_PI);
    if(hz > 3.0)
    {
        warn("speed=%.2f produces %.3f Hz RGB scrolling, "
                "which may trigger photosensitive epilepsy", speed, hz);
        info("recommended maximum speed=~%.2f", 3.0 * 2.0 * M_PI);
        if(!exu_ask_yes_no())
            return 0;
    }
 
    if (td_init() != TD_ERR_OK)
        return 1;
 
    window = td_window_create(
            param.pos.x, param.pos.y, 
            param.size.x, param.size.y, param.rotation, 
            TD_COLOR_TRUECOLOR, 
            (param.size.x == 0 || param.size.y == 0 ? TD_WINDOW_FULLSCREEN : 0) |
            (param.auto_resize ? TD_WINDOW_RESIZABLE : 0)
        );
    if(!window)
    {
        error("td_window_create failed");
        goto cleanup;
    }
    renderer = td_renderer_create(window); 
    if(!renderer)
    {
        error("td_renderer_create failed");
        goto cleanup;
    }
/*
    td_vtx_attr_t attribs[] = { TDVA_POSITION_2D, TDVA_UV_COORDS};
    td_font* font = td_default_font((td_rgba){255, 255, 255, 255}, (td_rgba){0});
*/
    target_dt = 1000 / 60;
    while (!td_window_should_close(window)) 
    {
        td_u64 frame_start = 0;
        td_u8 r = 0, g = 0, b = 0;
        char fps_string[64] = {0};

        frame_start = td_get_performance_counter();

        td_poll_events();

        r = (sin(elapsed) + 1) * 127.5;
        g = (sin(elapsed + (2 * M_PI / 3)) + 1) * 127.5;
        b = (sin(elapsed + (4 * M_PI / 3)) + 1) * 127.5;

        td_renderer_set_draw_color(renderer, r, g, b, 255);

        td_renderer_clear(renderer);
/*
        td_texture_t *texture =
            td_render_string(font, string, strlen(string));
        size = td_texture_get_size(texture);

        td_renderer_bind_texture(texture);
        td_ivec2 display_sz = {0};
        td_vec2 right_pos = pos_to_ndc((td_ivec2){.x=size.x} , display_sz);
        td_vec2 bottom_pos = pos_to_ndc((td_ivec2){.y=size.y}, display_sz);
        vertices[1 * 4 + 0] = right_pos.x;
        vertices[2 * 4 + 0] = right_pos.x;
        vertices[2 * 4 + 1] = bottom_pos.y;
        vertices[4 * 4 + 0] = right_pos.x;
        vertices[4 * 4 + 1] = bottom_pos.y;
        vertices[5 * 4 + 1] = bottom_pos.y;
        for(int i = 0; i < sizeof(vertices) / sizeof(float) / 4; i++)
            td_add_vertex(vertices + i * 4, attribs, sizeof(attribs) / sizeof(attribs[0]), TD_TRUE);
        td_texture_destroy(texture);
*/
        td_window_present(window);
        elapsed += ((double)dt / 1000.0) * speed;

        while ((dt = td_get_performance_counter() - frame_start) < target_dt)
            ;
    }

cleanup:
    td_renderer_destroy(renderer);
    td_window_destroy(window);
//    td_destroy_font(font);
    td_quit();
    return 0;
}
