#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef TD_PLATFORM_WINDOWS
#   include <bcrypt.h>
#endif

#include <aparse.h>
#include <td_main.h>

#include "example_utils.h"

static td_u8 desired_channel = 3;
td_texture *generate_noise(td_ivec2 size)
{
    td_texture *out = td_texture_create(0, desired_channel, size, 0, 0);
    td_u8 *raw = td_texture_get_pixel(out, (td_ivec2){0});
    td_u64 byte = size.x * size.y * desired_channel;
#ifdef TD_PLATFORM_WINDOWS
    if (BCryptGenRandom(0, raw, byte, BCRYPT_USE_SYSTEM_PREFERED_RNG)) {
        tdt_free(out);
        return 0;
    }
#else
    int fd = 0;
    if ((fd = open("/dev/urandom", O_RDONLY)) < 0) {
        td_texture_destroy(out);
        return 0;
    }
    if (read(fd, raw, byte) != byte) {
        close(fd);
        td_texture_destroy(out);
        return 0;
    }
    close(fd);
#endif
    return out;
}

int main(int argc, char** argv)
{
    example_params p = parse_argv(argc, argv, (aparse_arg[]){
        aparse_arg_option(0, "--desired-channel", &desired_channel, sizeof(desired_channel), APARSE_ARG_TYPE_UNSIGNED, "the number of channels for the noise")
    }, 1, 0);
    
    if(desired_channel == 0 || desired_channel > 4)
    {
        aparse_prog_error("invalid channels for the noise's texture");
        return 1;
    }
    td_u8 enable = 1;
    if (td_init() == td_false || start_logging("statics.txt"))
        return 1;

    use_params(&p);

    td_font* font = td_default_font((td_rgba){.a=255}, (td_rgba){{255,255,255,255}});

    td_ivec2 size = { 0 };    // Temporary
    double delta_time = 1.0, last_log = get_time();
    const double max_dt = 1.0 / p.max_fps;
    while (td_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        td_option(td_opt_display_size, 1, &size);
        td_texture *noise = generate_noise(size);
        td_copy_texture(noise, (td_ivec2){0});
        td_texture_destroy(noise);

        char *string = to_string("%f", fps);
        td_texture *texture =
            td_render_string(font, string, strlen(string));
        size = td_texture_get_size(texture);

        td_copy_texture(texture, (td_ivec2){0});
        td_texture_destroy(texture);

        td_render();

        while ((delta_time = get_time() - start_frame) < max_dt) {}
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();
        }
        free(string);
    }
    td_destroy_font(font);
    td_quit();
    stop_logging();
    return 0;
}
