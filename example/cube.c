#include <string.h>             // strlen
#include <stdlib.h>             // free
#include <cglm/cglm.h>

#include "term_display.h"
#include "term_font.h"

#include "example_utils.h"

static inline double gen_rand()
{
    return -1.0 + (1.0 - -1.0) * ((double) rand() / RAND_MAX);
}



float vertices[] = {
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,

    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,

    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,

    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f
};

int main()
{
    if (display_init() || start_logging("statics.txt"))
        return 1;

    term_ivec2 size = { 0 };
    double delta_time = 1.0, last_log = get_time();
    const double max_dt = 1.0 / 100000;
    while (display_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        display_poll_events();

        display_set_color(rgba_init(0, 0, 0, 255));

        char *string = to_string("%f", fps);
        term_texture *texture =
            display_string_texture(string, strlen(string), &size,
                                   rgba_init(255, 255, 255, 255),
                                   rgba_init(0, 0, 0, 0));
        display_copy_texture(texture, vec2_init(-1.0f, 1.0f),
                             TEXTURE_MERGE_CROP);
        texture_free(texture);

        term_vec2 p1 = vec2_init(gen_rand(), gen_rand()), p2 =
            vec2_init(gen_rand(), gen_rand()), p3 =
            vec2_init(gen_rand(), gen_rand());/*
        display_draw_line(p1, p2, rgba_init(0, 255, 255, 255));
        display_draw_line(p2, p3, rgba_init(0, 255, 255, 255));
        display_draw_line(p3, p1, rgba_init(0, 255, 255, 255));
        */
        display_draw_triangle(p1, p2, p3, rgba_init(0, 255, 255, 255));

//  display_option(settings_display_size, 1, &size);
//
//  mat4 model, view, projection, mvp;
//  glm_mat4_identity(model);
//  glm_mat4_identity(view);
//  glm_mat4_identity(projection);
//  glm_rotate(model, (f32)get_time(), (f32[3]){0.5f, 1.0, 0.0f});
//  glm_translate(view, (f32[3]){0.0f, 0.0f, -3.0f});
//  glm_perspective(glm_rad(45.0f), (f32)size.x/size.y, 0.1f, 100.0f, projection);
//
//  glm_mat4_mul(projection, view, mvp);
//  glm_mat4_mul(mvp, model, mvp);
//
//  for(int i = 0; i < sizeof(vertices) / sizeof(vertices[0]) / 3; i++)
//  {
//   vec3 out;
//   glm_mat4_mulv3(mvp, &vertices[i*3], 1.0f, out);
//   write_log("Line %d: { %f, %f, %f }", i, out[0], out[1], out[2]);
//  }

        display_show();

        while ((delta_time = get_time() - start_frame) < max_dt) {
        }
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();
        }
        free(string);
    }
    display_free();
    stop_logging();
    return 0;
}
