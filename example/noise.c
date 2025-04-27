#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "td_main.h"
#include "td_font.h"

#include "example_utils.h"

#ifdef TD_PLATFORM_WINDOWS
#include <bcrypt.h>
#endif

const term_u8 desired_channel = 4;
term_texture *generate_noise(term_ivec2 size)
{
    term_texture *out = tdt_create(0, desired_channel, size, 0, 0);
    term_u8 *raw = tdt_get_location(ivec2_init(0, 0), out);
    term_u64 byte = size.x * size.y * desired_channel;
#ifdef TD_PLATFORM_WINDOWS
    if (BCryptGenRandom(0, raw, byte, BCRYPT_USE_SYSTEM_PREFERED_RNG)) {
        tdt_free(out);
        return 0;
    }
#else
    int fd = 0;
    if ((fd = open("/dev/urandom", O_RDONLY)) < 0) {
        tdt_free(out);
        return 0;
    }
    if (read(fd, raw, byte) != byte) {
        close(fd);
        tdt_free(out);
        return 0;
    }
    close(fd);
#endif
    return out;
}

int main(int argc, char** argv)
{
    example_tdopt(argc, argv);
    term_u8 enable = 1;
    if (td_init() || start_logging("statics.txt"))
        return 1;

    term_ivec2 size = { 0 };    // Temporary
    double delta_time = 1.0, last_log = get_time();
    while (td_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        td_option(td_opt_display_size, 1, &size);
        term_texture *noise = generate_noise(size);
        td_copy_texture(noise, vec2_init(-1.0f, 1.0f),
                             TEXTURE_MERGE_CROP);
        tdt_free(noise);

        char *string = to_string("%f", fps);
        term_texture *texture =
            tdf_string_texture(string, strlen(string), &size,
                                   rgba_init(0, 0, 0, 255), rgba_init(255,
                                                                      255,
                                                                      255,
                                                                      255));
        td_copy_texture(texture, vec2_init(-1.0f, 1.0f),
                             TEXTURE_MERGE_CROP);
        tdt_free(texture);

        td_show();

        delta_time = get_time() - start_frame;
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
