#include <string.h>             // strlen
#include <stdlib.h>             // free
#include <math.h>               // sin

#include "td_main.h"
#include "td_font.h"

#include "example_utils.h"

#define STR_BUF_SIZE 10

char* buffer = 0;
int current_size = 0, current_index = 0;

td_texture* string_texture = 0;
void refresh_str_texture()
{
    if(string_texture) {
        tdt_free(string_texture);
        string_texture = 0;
    }
    string_texture = tdf_string_texture(buffer, -1, 
        0, td_rgba_init(255,255,255,255), td_rgba_init(0,0,0,0));
}


void process_input(int key, int mods, td_key_state_t actions)
{
    if(key == td_key_backspace)
    {
        if(current_index == 0) return;
        buffer[(current_index = current_index - 1)] = 0;
    }
    else if(key == td_key_enter)
    {
        buffer[current_index++] = '\n';
    }
    else {
        if(current_index + 1 >= current_size)
        {
            int new_size = current_size + STR_BUF_SIZE;
            char* tmp = (char*)realloc(buffer, new_size);
            if(!tmp) return;
            buffer = tmp;
            memset(&buffer[current_size], 0, STR_BUF_SIZE);
            current_size = new_size;
        }
        buffer[current_index++] = (char)key;
    }
    refresh_str_texture();
}

int main(int argc, char** argv)
{
    parse_argv(argc, argv, 0, 0);
    td_u8 enable = 1;
    if (td_init() || start_logging("statics.txt"))
        return 1;
    td_option(td_opt_auto_resize, 0, &enable);
    td_set_key_callback(process_input);

    td_ivec2 size = { 0 };
    td_u64 frame_count = 0;
    double delta_time = 1.0, last_log = get_time();
    const double max_dt = 1.0 / maximum_fps;
    while (td_is_running()) {
        frame_count++;
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        float brightness = 0.5f * (sin((double)frame_count / 32) + 1.0f);
        tdr_set_clear_color(td_rgba_init(109 * brightness, 154 * brightness, 140 * brightness, 255));   // Approximtely patina

        char *string = to_string("%f", fps);
        td_texture *texture =
            tdf_string_texture(string, strlen(string), &size,
                                td_rgba_init(0, 0, 0, 255), td_rgba_init(255, 255, 255, 255));
        tdr_copy_texture(texture, td_ivec2_init(0, 0));
        tdt_free(texture);

        tdr_copy_texture(string_texture, td_ivec2_init(0, size.y + 1));

        tdr_render();

        while ((delta_time = get_time() - start_frame) < max_dt) {}
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();
        }
        free(string);
    }
    tdt_free(string_texture);
    td_free();
    stop_logging();
    return 0;
}
