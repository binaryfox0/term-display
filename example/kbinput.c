#include <string.h>             // strlen, memcpy
#include <stdlib.h>             // free

#include "term_display.h"
#include "term_font.h"
#include "example_utils.h"

#define MAX_KEY_STR 48          // Define max key string length

static char key_pressed[MAX_KEY_STR] = "No key is pressed";

// Modifier key lookup table
static const struct {
    int mod;
    const char *text;
} mods_lookup[] = {
    { key_ctrl, "Ctrl + " },
    { key_alt, "Alt + " },
    { key_shift, "Shift + " }
};

// Function key names
static const char *fkey_name[] = {
    "F1", "F2", "F3", "F4", "F5", "F6",
    "F7", "F8", "F9", "F10", "F11", "F12"
};

// Key callback function
int prev_key = 0, counter = 0;
void key_callback(int key, int mods, key_state state)
{
    if (prev_key == key)
        counter++;
    else {
        prev_key = key;
        counter = 1;
    }
    write_log("Repeated key: %d", counter);
    int index = 0;

    // Add modifier keys if present
    for (size_t i = 0; i < sizeof(mods_lookup) / sizeof(mods_lookup[0]);
         i++) {
        if (mods & mods_lookup[i].mod) {
            size_t len = strlen(mods_lookup[i].text);
            memcpy(&key_pressed[index], mods_lookup[i].text, len);
            index += len;
        }
    }

    // Identify the pressed key
    if (IN_RANGE(key, term_key_astrophe, term_key_grave_accent)) {
        key_pressed[index++] = key;
    } else if (IN_RANGE(key, term_key_f1, term_key_f12)) {
        const char *key_name = fkey_name[key - term_key_f1];
        size_t len = strlen(key_name);
        memcpy(&key_pressed[index], key_name, len);
        index += len;
    } else {
        const char *key_name = NULL;

        switch (key) {
        case term_key_space:
            key_name = "Space";
            break;
        case term_key_backspace:
            key_name = "Back";
            break;
        case term_key_escape:
            key_name = "Esc";
            break;
        case term_key_tab:
            key_name = "Tab";
            break;
        case term_key_enter:
            key_name = "Enter";
            break;
        case term_key_home:
            key_name = "Home";
            break;
        case term_key_insert:
            key_name = "Ins";
            break;
        case term_key_delete:
            key_name = "Del";
            break;
        case term_key_end:
            key_name = "End";
            break;
        case term_key_page_up:
            key_name = "PgUp";
            break;
        case term_key_page_down:
            key_name = "PgDn";
            break;
        case term_key_up:
            key_name = "Up";
            break;
        case term_key_left:
            key_name = "Left";
            break;
        case term_key_down:
            key_name = "Down";
            break;
        case term_key_right:
            key_name = "Right";
            break;
        }

        if (key_name) {
            size_t len = strlen(key_name);
            memcpy(&key_pressed[index], key_name, len);
            index += len;
        }
    }

    // Append " is pressed."
    memcpy(&key_pressed[index], " is pressed.", 13);
}

// Main function
int main()
{
    u8 enable = 1;

    if (display_init() || start_logging("statics.txt"))
        return 1;

    display_option(settings_auto_resize, 0, &enable);
    display_set_key_callback(key_callback);

    u64 frame_count = 0;
    term_texture *texture = NULL;
    term_ivec2 size = { 0 };
    double delta_time = 1.0, last_log = get_time();

    while (display_is_running()) {
        frame_count++;
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        display_set_color(rgba_init(109, 154, 140, 255));       // Patina color
        display_poll_events();

        // FPS Display
        char *fps_str = to_string("%f", fps);
        texture = display_string_texture(fps_str, strlen(fps_str), &size,
                                         rgba_init(255, 255, 255, 255),
                                         rgba_init(0, 0, 0, 0));
        display_copy_texture(texture, vec2_init(-1.0f, 1.0f),
                             TEXTURE_MERGE_RESIZE);
        texture_free(texture);

        // Key Press Display
        texture =
            display_string_texture(key_pressed, strlen(key_pressed), &size,
                                   rgba_init(255, 255, 255, 255),
                                   rgba_init(0, 0, 0, 0));
        display_copy_texture(texture, vec2_init(-1.0f, 0.0f),
                             TEXTURE_MERGE_CROP);
        texture_free(texture);

        display_show();

        delta_time = get_time() - start_frame;
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", fps_str);
            last_log = get_time();
        }

        free(fps_str);
    }

    display_free();
    stop_logging();
    return 0;
}
