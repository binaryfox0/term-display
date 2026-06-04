#include <td_main.h>
#include <aparse.h>

/*
 int main(int argc, char **argv)
{
    td_window_t *window = 0;
    td_renderer_t *renderer = 0;

    window = td_window_create(
            0, 0, 0, 0, 0,
            TD_COLOR_TRUECOLOR,
            TD_WINDOW_FULLSCREEN | TD_WINDOW_RESIZABLE
    );

    if(!window)
        return 1;

    renderer = td_renderer_create(window);
    if(!renderer)
        return 1;

    int mode = 0;
    double t = 0.0;

    while(!td_window_should_close(window))
    {
        td_poll_events(window);

        td_ivec2 sz = td_renderer_get_size(renderer);

        td_renderer_clear(renderer);

        if(mode == 0)
        {
            // diagonal X line
            td_renderer_draw_line(renderer,
                    0, 0,
                    sz.x - 1, sz.y - 1,
                    (td_rgba){255, 0, 0, 255});

            td_renderer_draw_line(renderer,
                    sz.x - 1, 0,
                    0, sz.y - 1,
                    (td_rgba){0, 255, 0, 255});
        }
        else if(mode == 1)
        {
            // grid test (rotation killer)
            for(int y = 0; y < sz.y; y += 4)
            {
                for(int x = 0; x < sz.x; x += 4)
                {
                    td_renderer_draw_point(renderer,
                            x, y,
                            (td_rgba){x % 255, y % 255, (x ^ y) % 255, 255});
                }
            }
        }

        td_renderer_present(renderer);
    }

cleanup:
    td_renderer_destroy(renderer);
    td_window_destroy(window);

    return 0;
}
*/
int main(int argc, char **argv)
{
    td_i32 rotation = 0;
    aparse_arg main_args[] = {
        aparse_arg_option(
                "-rot", "--rotation",
                &rotation, sizeof(rotation),
                APARSE_ARG_TYPE_SIGNED, 0),
        aparse_arg_end_marker
    };
    td_window_t *window = 0;
    td_renderer_t *renderer = 0;

    if(aparse_parse(argc, argv, main_args, 0, 0) != APARSE_STATUS_OK)
        return 1;

    window = td_window_create(
            0, 0, 0, 0, rotation,
            TD_COLOR_TRUECOLOR,
            TD_WINDOW_FULLSCREEN | TD_WINDOW_RESIZABLE
    );

    renderer = td_renderer_create(window);

    while(!td_window_should_close(window))
    {
        td_i32 width = 0, height = 0;

        td_poll_events(window);
        td_renderer_get_size(renderer, &width, &height);

        // fill entire framebuffer
        for(td_i32 y = 0; y < height; y++)
        {
            for(td_i32 x = 0; x < width; x++)
            {
                td_renderer_set_draw_color(
                        renderer,
                        (td_u8)((x * 255) / (width ? width : 1)),
                        (td_u8)((y * 255) / (height ? height : 1)),
                        (td_u8)((x ^ y) & 255),
                        255
                );

                td_renderer_draw_point(renderer, x, y);
            }
        }

        // top, bottom border
        for(td_i32 x = 0; x < width; x++)
        {
            // right
            td_renderer_set_draw_color(renderer, 
                    255, 0, 0, 255);
            td_renderer_draw_point(renderer, x, 0); 
            td_renderer_set_draw_color(renderer, 
                    0, 255, 0, 255);
            td_renderer_draw_point(renderer, x, height - 1);
        }

        // left, right border
        for(td_i32 y = 0; y < height; y++)
        {
            // left - blue
            td_renderer_set_draw_color(renderer, 
                    0, 0, 255, 255);
            td_renderer_draw_point(renderer, 0, y);
            // right - white
            td_renderer_set_draw_color(renderer, 
                    255, 255, 255, 255);
            td_renderer_draw_point(renderer, width - 1, y);
        }

        // top-left corner
        td_renderer_set_draw_color(renderer, 
                255, 255, 0, 255);
        td_renderer_draw_point(renderer, 0, 0);
        // top-right corner
        td_renderer_set_draw_color(renderer, 
                255, 0, 255, 255);
        td_renderer_draw_point(renderer, width - 1, 0); 
        // bottom-left corner
        td_renderer_set_draw_color(renderer, 
                0, 255, 255, 255);
        td_renderer_draw_point(renderer, 0, height - 1);
        // bottom-right corner
        td_renderer_set_draw_color(renderer, 
                255, 255, 255, 255);
        td_renderer_draw_point(renderer, width - 1, height - 1);
        td_window_present(window);
    }

cleanup:
    td_renderer_destroy(renderer);
    td_window_destroy(window);

    return 0;
}
