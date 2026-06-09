#include <stdio.h>

#include <td_main.h>
#include <aparse.h>

#define error aparse_prog_error
#define info aparse_prog_info

#include <stdio.h>

typedef struct fps_counter_t
{
    td_u64 last_counter;
    td_u64 freq;
    double accum_time;
    td_u32 frame_count;
    double fps;
} fps_counter_t;

void fps_counter_init(fps_counter_t *fc)
{
    fc->last_counter = td_get_performance_counter();
    fc->freq = td_get_performance_frequency();
    fc->accum_time = 0.0;
    fc->frame_count = 0;
    fc->fps = 0.0f;
}

void fps_counter_update(fps_counter_t *fc)
{
    td_u64 now = 0;
    double elapsed_sec = 0.0;

    now = td_get_performance_counter();

    fc->frame_count++;

    elapsed_sec = (double)(now - fc->last_counter) / (double)fc->freq;

    fc->accum_time += elapsed_sec;
    fc->last_counter = now;

    if (fc->accum_time >= 1.0 / 60)
    {
        fc->fps = (double)fc->frame_count / (float)fc->accum_time;

        td_debug_log("fps=%.2f", fc->fps);

        fc->frame_count = 0;
        fc->accum_time -= 1.0;
    }
}

void key_callback(
        td_window_t *window,
        td_key_token_t key,
        td_key_action_t action,
        td_key_mod_t mod)
{
    switch(key)
    {
        case TD_KEY_S:
        {
            td_window_flags_t flags = 0;

            if(mod != TD_MOD_NONE)
                break;

            flags = td_window_get_flags(window);

            td_window_set_resizable(
                    window,
                    !(flags & TD_WINDOW_RESIZABLE));
            return;
        }

        case TD_KEY_R:
            if(mod != TD_MOD_NONE)
                break;

            td_window_set_orientation(
                    window,
                    td_window_get_orientation(window) + 1);
            return;

        case TD_KEY_UP:
        case TD_KEY_DOWN:
        case TD_KEY_LEFT:
        case TD_KEY_RIGHT:
        {
            td_i32 x = 0;
            td_i32 y = 0;

            if(mod != TD_MOD_NONE)
                break;

            td_window_get_position(window, &x, &y);

            switch(key)
            {
                case TD_KEY_UP:
                    y--;
                    break;
                case TD_KEY_DOWN:
                    y++;
                    break;
                case TD_KEY_LEFT:
                    x--;
                    break;
                case TD_KEY_RIGHT:
                    x++;
                    break;
                default:
                    return;
            }

            td_window_set_position(window, x, y);
            return;
        }

        case TD_KEY_MINUS:
        {
            td_i32 width = 0;
            td_i32 height = 0;

            if(mod != TD_MOD_NONE)
                break;

            td_window_get_size(window, &width, &height);

            td_window_set_size(
                    window,
                    width - 1,
                    height - 1);
            return;
        }

        case TD_KEY_EQUAL:
        {
            td_i32 width = 0;
            td_i32 height = 0;

            if(mod != TD_MOD_SHIFT)
                break;

            td_window_get_size(window, &width, &height);

            td_window_set_size(
                    window,
                    width + 1,
                    height + 1);
            return;
        }

        default:
            return;
    }
}

int main(int argc, char **argv)
{
    const char *size = 0;
    aparse_arg main_args[] = {
        aparse_arg_option(
                "--size", "--initial-size",
                &size, 0,
                APARSE_ARG_TYPE_STRING, 0),
        aparse_arg_end_marker
    };
    
    td_error_t err = TD_ERR_OK;
    td_i32 width = 0, height = 0;
    td_window_t *window = 0;
    td_renderer_t *renderer = 0;
    fps_counter_t fc = {0};

    if(aparse_parse(argc, argv, main_args, 
                0, 0) != APARSE_STATUS_OK)
        return 1;

    if(size &&
            sscanf(size, "%dx%d", &width, &height) != 2)
    {
        error("invalid string was given for paramater size");
        return 1;
    }

    err = td_init();
    if(err != TD_ERR_OK)
    {
        error("failed to initialize term-display");
        info("info: \"%s\"", td_strerror(err));
        return 1;
    }
    td_debug_set("static_debug.txt");
    window = td_window_create(
            0, 0, width, height, 0,
            TD_COLOR_TRUECOLOR,
            width != 0 && height != 0 ? 0 : TD_WINDOW_FULLSCREEN
    );
   
    td_set_key_callback(window, key_callback);

    renderer = td_renderer_create(window);
    fps_counter_init(&fc);
    while(!td_window_should_close(window))
    {
        td_i32 width = 0, height = 0;

        td_poll_events();
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
            // top - red
            td_renderer_set_draw_color(renderer, 
                    255, 0, 0, 255);
            td_renderer_draw_point(renderer, x, 0); 
            // bottm - green
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
        fps_counter_update(&fc);
    }

cleanup:
    td_renderer_destroy(renderer);
    td_window_destroy(window);
    td_quit();

    return 0;
}
