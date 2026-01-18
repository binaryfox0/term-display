#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "td_main.h"
#include "td_font.h"

#include "example_utils.h"
#include "td_renderer.h"
#include "td_texture.h"

// #define CURSOR_SIZE 3

#define WHITE 0xff, 0xff
#define GRAY  0x00, 0x80
#define BLACK 0x00, 0xff
#define EMPTY 0x00, 0x00

#define BGCOL TD_RGBA(0xefe6d8ff)
#define FGCOL TD_RGBA(0x3a3a36ff)

static td_ivec2 mousepos = {0}, mousepos_prev = {0};
static td_ivec2 cursorpos_textsz ={0}, cursorkey_textsz = {0};
static td_texture
    *cursor_tex = 0,
    *cursorpos_tex = 0,
    *cursorkey_tex = 0;
static td_font *font = 0;

void refresh_texture(td_texture** tex, const char* str, td_ivec2* texsz)
{
    if(!tex) return;
    if(*tex) {
        td_texture_destroy(*tex);
        *tex = 0;
    }

    *tex = td_render_string(font, str, -1, texsz);
}

static int repeat_count = 0, prev_key = -1;
void mousecb(int xpos, int ypos, int key)
{
    if(prev_key != key || 
            mousepos_prev.x != xpos ||
            mousepos_prev.y != ypos)
    {
        repeat_count = 0;
        prev_key = key;
        mousepos_prev = (td_ivec2){.x=xpos,.y=ypos};
    } else {
        repeat_count++;
    }
    mousepos = (td_ivec2){.x=xpos, .y=ypos};
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%d, %d", xpos, ypos);
    refresh_texture(&cursorpos_tex, buf, &cursorpos_textsz);
    snprintf(buf, sizeof(buf), "%d x %d", key, repeat_count);
    refresh_texture(&cursorkey_tex, buf, &cursorkey_textsz);
}

int load_cursor()
{
    // grayscale, alpha
    uint8_t bitmap[] = {
        BLACK, BLACK, GRAY,  EMPTY, EMPTY, EMPTY, EMPTY,
        BLACK, WHITE, BLACK, GRAY,  EMPTY, EMPTY, EMPTY,
        BLACK, WHITE, WHITE, BLACK, GRAY,  EMPTY, EMPTY,
        BLACK, WHITE, WHITE, WHITE, BLACK, GRAY,  EMPTY,
        BLACK, WHITE, WHITE, WHITE, WHITE, BLACK, GRAY,
        BLACK, WHITE, WHITE, WHITE, WHITE, GRAY,  BLACK,
        BLACK, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK,
        BLACK, GRAY,  BLACK, WHITE, WHITE, BLACK, GRAY,
        BLACK, BLACK, BLACK, GRAY,  WHITE, BLACK, EMPTY,
        EMPTY, EMPTY, GRAY,  BLACK, BLACK, GRAY, EMPTY
    };
    cursor_tex = td_texture_create(bitmap, 2, (td_ivec2){.x=7,.y=10}, 
            td_false, td_true);
    if(!cursor_tex)
        return -1;
    td_texture_convert(cursor_tex, 4);
    return 0;
}

int main(int argc, char** argv)
{
    example_params p = parse_argv(argc, argv, 0, 0, 0);
    td_u8 enable = 1;
    if (td_init() == td_false || start_logging("statics.txt"))
        return 1;
   
    if(load_cursor() == -1)
        goto cleanup;

    use_params(p);

    td_set_mouse_callback(mousecb);
    font = td_default_font(FGCOL, BGCOL);

    td_ivec2 size = { 0 };    // Temporary
    double delta_time = 1.0, last_log = get_time();
    const double max_dt = 1.0 / p.max_fps;
    while (td_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        td_set_clear_color(BGCOL);

        td_ivec2 fbsz = (td_ivec2){0};
        td_option(td_opt_display_size, td_true, &fbsz);

        char *string = to_string("%f", fps);
        td_texture *texture =
            td_render_string(font, string, strlen(string), &size);
        td_copy_texture(texture, (td_ivec2){0});
        td_texture_destroy(texture);

        td_copy_texture(cursorpos_tex, (td_ivec2){.x=0, .y=fbsz.y - cursorpos_textsz.y});
        td_copy_texture(cursorkey_tex, (td_ivec2){.x=fbsz.x - cursorkey_textsz.x, .y=fbsz.y - cursorkey_textsz.y});

        td_copy_texture(cursor_tex, mousepos);
        // td_draw_rect(mousepos, 
        //               (td_ivec2){.x=mousepos.x+CURSOR_SIZE,.y=mousepos.y+CURSOR_SIZE},
        //               (td_rgba){.r = 255, .a = 255});

        td_render();

        while ((delta_time = get_time() - start_frame) < max_dt) {}
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();
        }
        free(string);
    }

cleanup:
    td_texture_destroy(cursor_tex);
    td_texture_destroy(cursorpos_tex);
    td_texture_destroy(cursorkey_tex);
    td_destroy_font(font);
    
    td_quit();
    stop_logging();
    return 0;
}
