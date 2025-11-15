#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "td_main.h"
#include "td_font.h"

#include "example_utils.h"

#ifdef TD_PLATFORM_WINDOWS
#include <bcrypt.h>
#endif

const td_u8 desired_channel = 3;
td_texture *generate_noise(td_ivec2 size)
{
    td_texture *out = tdt_create(0, desired_channel, size, 0, 0);
    td_u8 *raw = tdt_get_location((td_ivec2){0}, out);
    td_u64 byte = size.x * size.y * desired_channel;
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
    example_params p = parse_argv(argc, argv, 0, 0, 0);
    td_u8 enable = 1;
    if (td_init() || start_logging("statics.txt"))
        return 1;

    use_params(p);

    td_ivec2 size = { 0 };    // Temporary
    double delta_time = 1.0, last_log = get_time();
    const double max_dt = 1.0 / p.max_fps;
    while (td_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        td_option(td_opt_display_size, 1, &size);
        td_texture *noise = generate_noise(size);
        tdr_copy_texture(noise, (td_ivec2){0});
        tdt_free(noise);

        char *string = to_string("%f", fps);
        td_texture *texture =
            tdf_string_texture(string, strlen(string), &size,
                                (td_rgba){.a=255}, (td_rgba){255, 255, 255, 255});
        tdr_copy_texture(texture, (td_ivec2){0});
        tdt_free(texture);

        tdr_render();

        while ((delta_time = get_time() - start_frame) < max_dt) {}
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
