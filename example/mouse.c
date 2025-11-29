#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "td_main.h"
#include "td_font.h"

#include "example_utils.h"
#include "td_renderer.h"
#include "td_texture.h"

#define CURSOR_SIZE 3

static int mouse_xpos = 0, mouse_ypos = 0;
static td_ivec2 textsz = (td_ivec2){0};
static td_texture* cursorpos_tex = 0;
static td_font *font = 0;

void mousecb(int xpos, int ypos, int key)
{
    mouse_xpos = xpos;
    mouse_ypos = ypos;
    if(cursorpos_tex)
        td_texture_destroy(cursorpos_tex);
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%d, %d", xpos, ypos);
    cursorpos_tex = td_render_string(font, buf, sizeof(buf), &textsz);
}

int main(int argc, char** argv)
{
    example_params p = parse_argv(argc, argv, 0, 0, 0);
    td_u8 enable = 1;
    if (td_init() || start_logging("statics.txt"))
        return 1;

    use_params(p);

    td_set_mouse_callback(mousecb);
    font = td_default_font((td_rgba){.a=255}, (td_rgba){{255,255,255,255}});

    td_ivec2 size = { 0 };    // Temporary
    double delta_time = 1.0, last_log = get_time();
    const double max_dt = 1.0 / p.max_fps;
    while (td_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        tdr_clear_framebuffer();

        td_ivec2 fbsz = (td_ivec2){0};
        td_option(td_opt_display_size, td_true, &fbsz);

        char *string = to_string("%f", fps);
        td_texture *texture =
            td_render_string(font, string, strlen(string), &size);
        tdr_copy_texture(texture, (td_ivec2){0});
        td_texture_destroy(texture);

        tdr_draw_rect((td_ivec2){.x = mouse_xpos, .y = mouse_ypos}, 
                      (td_ivec2){.x = mouse_xpos + CURSOR_SIZE, .y = mouse_ypos + CURSOR_SIZE},
                      (td_rgba){.r = 255, .a = 255});

        tdr_copy_texture(cursorpos_tex, (td_ivec2){.x=0, .y=fbsz.y - textsz.y});

        tdr_render();

        while ((delta_time = get_time() - start_frame) < max_dt) {}
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();
        }
        free(string);
    }
    td_texture_destroy(cursorpos_tex);
    td_destroy_font(font);
    td_free();
    stop_logging();
    return 0;
}
