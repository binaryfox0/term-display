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

vec3 cameraPos   = (vec3){0.0f, 0.0f,  3.0f};
vec3 cameraFront = (vec3){0.0f, 0.0f, -1.0f};
vec3 cameraUp    = (vec3){0.0f, 1.0f,  0.0f};

double delta_time = 0.0f;

void processInput(int key, int mods, key_state action);

int main()
{
    if (display_init() || start_logging("statics.txt"))
        return 1;

    u8 enable = 1;
    display_option(settings_depth_buffer, 0, &enable);
    enable = display_grayscale_256;
    display_option(settings_display_type, 0, &enable);
    display_set_key_callback(processInput);

    term_ivec2 size = { 0 };
    float out[sizeof(vertices) / sizeof(vertices[0])] = {};
    double last_log = get_time();
    const double max_dt = 1.0 / 60;
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

        //term_vec2 p1 = vec2_init(gen_rand(), gen_rand()), p2 =
        //    vec2_init(gen_rand(), gen_rand()), p3 =
        //    vec2_init(gen_rand(), gen_rand());
        //display_draw_line(p1, p2, rgba_init(0, 255, 255, 255));
        //display_draw_line(p2, p3, rgba_init(0, 255, 255, 255));
        //display_draw_line(p3, p1, rgba_init(0, 255, 255, 255));
        //display_draw_triangle(p1, p2, p3, rgba_init(0, 255, 255, 255));

        display_option(settings_display_size, 1, &size);
            
        mat4 model, view, projection, mvp;
        glm_mat4_identity(model);
        glm_mat4_identity(view);
        glm_mat4_identity(projection);
        glm_rotate(model, (f32)get_time(), (f32[3]){0.5f, 1.0, 0.0f});
        vec3 center;
        glm_vec3_add(cameraPos, cameraFront, center);  // center = cameraPos + cameraFront
        glm_lookat(cameraPos, center, cameraUp, view);
        glm_perspective(glm_rad(45.0f), (f32)size.x/size.y, 0.1f, 100.0f, projection);
            
        glm_mat4_mul(projection, view, mvp);
        glm_mat4_mul(mvp, model, mvp);
            
        for (int i = 0; i < sizeof(vertices) / sizeof(vertices[0]) / 3; i++)
        {
            float *dest = &out[i * 3];
            vec4 transformed = {vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2], 1.0f};
            
            glm_mat4_mulv(mvp, transformed, transformed);  // Multiply by MVP matrix

            display_render_add(transformed, 4);
        }

        char *tmp = to_string("%.2f, %.2f, %.2f", cameraPos[0], cameraPos[1], cameraPos[2]);
        texture = display_string_texture(tmp, -1, &size, rgba_init(255,0,0,255), rgba_init(0,0,0,0));
        display_copy_texture(texture, vec2_init(-1.0f, 0.0f), TEXTURE_MERGE_RESIZE);
        texture_free(texture);
        free(tmp);

        display_show();

        while ((delta_time = get_time() - start_frame) < max_dt) {
        }
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();
            for(int i = 0; i < sizeof(vertices) / sizeof(vertices[0]) / 3; i++)
                write_log("Vertex %d: [%f, %f, %f]", i, out[i*3], out[i*3+1], out[i*3+2]);

        }
        free(string);
    }
    display_free();
    stop_logging();
    return 0;
}

void processInput(int key, int mods, key_state action)
{
    float cameraSpeed = 10.0f * delta_time;
    vec3 right, tmp;

    if (key == term_key_w && action == key_press) {
        glm_vec3_scale(cameraFront, cameraSpeed, tmp);
        glm_vec3_add(cameraPos, tmp, cameraPos);
    }
    if (key == term_key_s && action == key_press) {
        glm_vec3_scale(cameraFront, cameraSpeed, tmp);
        glm_vec3_sub(cameraPos, tmp, cameraPos);
    }
    if (key == term_key_a && action == key_press) {
        glm_vec3_cross(cameraFront, cameraUp, right);
        glm_vec3_normalize(right);
        glm_vec3_scale(right, cameraSpeed, tmp);
        glm_vec3_sub(cameraPos, tmp, cameraPos);
    }
    if (key == term_key_d && action == key_press) {
        glm_vec3_cross(cameraFront, cameraUp, right);
        glm_vec3_normalize(right);
        glm_vec3_scale(right, cameraSpeed, tmp);
        glm_vec3_add(cameraPos, tmp, cameraPos);
    }
}