#include <string.h>             // strlen
#include <stdlib.h>             // free
#include <cglm/cglm.h>

#include "td_main.h"
#include "td_font.h"

#include "example_utils.h"

float vertices[] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, 0.5f, -0.5f,
     0.5f, 0.5f, -0.5f,
     0.5f, 0.5f,  0.5f,
     0.5f, 0.5f,  0.5f,
    -0.5f, 0.5f,  0.5f,
    -0.5f, 0.5f, -0.5f
};

vec3 cameraPos   = (vec3){0.0f, 0.0f,  3.0f};
vec3 cameraFront = (vec3){0.0f, 0.0f, -1.0f};
vec3 cameraUp    = (vec3){0.0f, 1.0f,  0.0f};

double delta_time = 0.0f;

void processInput(int key, int mods, td_key_state_t action);

int main(int argc, char** argv)
{
    if (td_init() || start_logging("statics.txt"))
        return 1;

    // td_ivec2 pos = td_ivec2_init(14, 10);
    // td_option(td_opt_display_pos, 0, &pos);
    td_u8 rotation = 1;
    td_option(td_opt_display_rotate, 0, &rotation);
    td_u8 enable = 1;
    td_option(td_opt_depth_buffer, 0, &enable);
    td_option(td_opt_shift_translate, 0, &enable);
    td_set_key_callback(processInput);

    td_ivec2 size = { 0 };
    float out[sizeof(vertices) / sizeof(vertices[0])] = {};
    double last_log = get_time();
    const double max_dt = 1.0 / maximum_fps;
    while (td_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        td_set_color(rgba_init(0, 0, 0, 255));

        char *string = to_string("%f", fps);
        td_texture *texture =
            tdf_string_texture(string, strlen(string), &size,
                                   rgba_init(255, 255, 255, 255),
                                   rgba_init(0, 0, 0, 0));
        td_copy_texture(texture, td_vec2_init(-1.0f, 1.0f),
                             TEXTURE_MERGE_CROP);
        tdt_free(texture);

        //td_vec2 p1 = td_vec2_init(gen_rand(), gen_rand()), p2 =
        //    td_vec2_init(gen_rand(), gen_rand()), p3 =
        //    td_vec2_init(gen_rand(), gen_rand());
        //td_draw_line(p1, p2, rgba_init(0, 255, 255, 255));
        //td_draw_line(p2, p3, rgba_init(0, 255, 255, 255));
        //td_draw_line(p3, p1, rgba_init(0, 255, 255, 255));
        //td_draw_triangle(p1, p2, p3, rgba_init(0, 255, 255, 255));

        td_option(td_opt_display_size, 1, &size);
            
        mat4 model, view, projection, mvp;
        glm_mat4_identity(model);
        glm_mat4_identity(view);
        glm_mat4_identity(projection);
        glm_rotate(model, (td_f32)get_time(), (td_f32[3]){0.5f, 1.0, 0.0f});
        vec3 center;
        glm_vec3_add(cameraPos, cameraFront, center);  // center = cameraPos + cameraFront
        glm_lookat(cameraPos, center, cameraUp, view);
        glm_perspective(glm_rad(45.0f), (td_f32)size.x/size.y, 0.1f, 100.0f, projection);
            
        glm_mat4_mul(projection, view, mvp);
        glm_mat4_mul(mvp, model, mvp);
            
        for (int i = 0; i < sizeof(vertices) / sizeof(vertices[0]) / 3; i++)
        //for(int i = 0; i < 3; i++)
        {
            float *dest = &out[i * 3];
            vec4 transformed = {vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2], 1.0f};
            
            glm_mat4_mulv(mvp, transformed, transformed);  // Multiply by MVP matrix

            td_render_add(transformed, 4);
        }

        char *tmp = to_string("%.2f, %.2f, %.2f", cameraPos[0], cameraPos[1], cameraPos[2]);
        texture = tdf_string_texture(tmp, -1, &size, rgba_init(255,0,0,255), rgba_init(0,0,0,0));
        td_copy_texture(texture, td_vec2_init(-1.0f, 0.0f), TEXTURE_MERGE_RESIZE);
        tdt_free(texture);
        free(tmp);

        td_show();

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

void processInput(int key, int mods, td_key_state_t action)
{
    float cameraSpeed = 10.0f * delta_time;
    vec3 right, tmp;

    if (key == td_key_w && action == td_key_press) {
        glm_vec3_scale(cameraFront, cameraSpeed, tmp);
        glm_vec3_add(cameraPos, tmp, cameraPos);
    }
    if (key == td_key_s && action == td_key_press) {
        glm_vec3_scale(cameraFront, cameraSpeed, tmp);
        glm_vec3_sub(cameraPos, tmp, cameraPos);
    }
    if (key == td_key_a && action == td_key_press) {
        glm_vec3_cross(cameraFront, cameraUp, right);
        glm_vec3_normalize(right);
        glm_vec3_scale(right, cameraSpeed, tmp);
        glm_vec3_sub(cameraPos, tmp, cameraPos);
    }
    if (key == td_key_d && action == td_key_press) {
        glm_vec3_cross(cameraFront, cameraUp, right);
        glm_vec3_normalize(right);
        glm_vec3_scale(right, cameraSpeed, tmp);
        glm_vec3_add(cameraPos, tmp, cameraPos);
    }
}