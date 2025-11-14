#include <string.h>             // strlen
#include <stdlib.h>             // free
#include <math.h>               // sin

#include "td_main.h"
#include "td_font.h"

#include "example_utils.h"

#define KSTROK_BUFSIZ 48

int kstork_texh = 0;

char* buffer = 0;
int current_size = 0, current_index = 0;

static const struct {
    int mod;
    const char *text;
} mods_lookup[] = {
    { td_key_ctrl, "Ctrl + " },
    { td_key_alt, "Alt + " },
    { td_key_shift, "Shift + " }
};

static const char *fkey_name[] = {
    "F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"
};

td_texture* textinput_tex = 0;
td_texture* keystroke_tex = 0;

td_ivec2 fbsz = {0};

void refresh_texture(td_texture** tex, const char* str, td_ivec2* texsz)
{
    if(!tex) return;
    if(*tex) {
        tdt_free(*tex);
        *tex = 0;
    }

    *tex = tdf_string_texture(str, -1, 
        texsz, td_rgba_init(255,255,255,255), td_rgba_init(0,0,0,0));
}

void append_char_textin(const int key)
{
    if(key == td_key_backspace)
    {
        if(current_index == 0) return;
        buffer[(current_index = current_index - 1)] = '\0';
        if(current_index < current_size / 2 - 1 &&  current_size > 4)
        {
            int new_size = current_size / 2;
            char* tmp = (char*)realloc(buffer, new_size);
            if(!tmp) return;
            buffer = tmp;
            current_size = new_size;
        }
    }
    else if(key == td_key_enter)
    {
        buffer[current_index++] = '\n';
    }
    else {
        if(current_index + 1 >= current_size)
        {
            int new_size = current_size == 0 ? 4 : current_size * 2;
            char* tmp = (char*)realloc(buffer, new_size);
            if(!tmp) return;
            buffer = tmp;
            memset(&buffer[current_size], 0, new_size - current_size);
            current_size = new_size;
        }
        buffer[current_index++] = (char)key;
    }
    refresh_texture(&textinput_tex, buffer, 0);
}

const char* map_special_key(const int key)
{
    switch (key) {
        case td_key_space:       return "Space";
        case td_key_backspace:   return "Back";
        case td_key_escape:      return "Esc";
        case td_key_tab:         return "Tab";
        case td_key_enter:       return "Enter";
        case td_key_home:        return "Home";
        case td_key_insert:      return "Ins";
        case td_key_delete:      return "Del";
        case td_key_end:         return "End";;
        case td_key_page_up:     return "PgUp";
        case td_key_page_down:   return "PgDn";
        case td_key_up:          return "Up";
        case td_key_left:        return "Left";
        case td_key_down:        return "Down";
        case td_key_right:       return "Right";
    }
    return ""; 
}

void build_keystrok_str(const int key, const int mods)
{
    static int repeat_count = 0;
    static int prev_k = 0, prev_mods = 0;

    char buf[KSTROK_BUFSIZ] = {0};
    td_ivec2 texsz = {0};

    if(prev_k == key && prev_mods == mods)
        repeat_count++;
    else
        prev_k = key, prev_mods = mods, repeat_count = 1;

    char *p = buf;
    char *end = buf + sizeof(buf) - 1;

    // --- Append Modifiers ---
    for (size_t i = 0; i < sizeof(mods_lookup)/sizeof(mods_lookup[0]); i++) {
        if (mods & mods_lookup[i].mod) {
            const char *m = mods_lookup[i].text;
            while (*m && p < end) *p++ = *m++;
        }
    }

    const char *name = NULL;

    if (IN_RANGE(key, '!', '~')) {
        if (p < end) *p++ = (char)key;
    }
    else if (IN_RANGE(key, td_key_f1, td_key_f12)) {
        name = fkey_name[key - td_key_f1];
    }
    else {
        name = map_special_key(key);
    } 

    if (name) {
        while (*name && p < end) *p++ = *name++;
    }

    snprintf(p, end - p, " x %d", repeat_count);

    *(end - 1) = '\0';
    
    refresh_texture(&keystroke_tex, buf, &texsz);
    kstork_texh = texsz.y;
}

void process_input(int key, int mods, td_key_state_t actions)
{
    append_char_textin(key);
    build_keystrok_str(key, mods);
}

void resize_handle(td_ivec2 new_size) {
    fbsz = new_size;
}

int main(int argc, char** argv)
{
    example_params p = parse_argv(argc, argv, 0, 0, 0);
    td_bool enable = td_false;

    if (td_init() || start_logging("statics.txt"))
        return 1;

    use_params(p);

    td_option(td_opt_shift_translate, td_false, &enable);
    td_option(td_opt_display_size, td_true, &fbsz);
    td_set_key_callback(process_input);
    td_set_resize_callback(resize_handle);

    td_ivec2 size = { 0 };
    td_u64 frame_count = 0;
    double delta_time = 1.0, last_log = get_time();
    const double max_dt = 1.0 / p.max_fps;
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

        tdr_copy_texture(textinput_tex, td_ivec2_init(0, size.y + 1));
        tdr_copy_texture(keystroke_tex, td_ivec2_init(0, fbsz.y - kstork_texh));

        tdr_render();

        while ((delta_time = get_time() - start_frame) < max_dt) {}
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();
        }
        free(string);
    }

    tdt_free(textinput_tex);
    tdt_free(keystroke_tex);

    td_free();
    stop_logging();
    return 0;
}
