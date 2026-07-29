#include <stdlib.h>             // free

#include <td_main.h>
#include <cglm/cglm.h>

#include "example_utils.h"

static const float vertices[] = {
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

static const float vertex_colors[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

static vec3 cameraPos   = (vec3){0.0f, 0.0f,  3.0f};
static vec3 cameraFront = (vec3){0.0f, 0.0f, -1.0f};
static vec3 cameraUp    = (vec3){0.0f, 1.0f,  0.0f};

static double delta_time = 0.0f;
static void key_callback(
        td_key_token_t key, 
        td_key_action_t action, 
        td_key_mod_t mods)
{
    float cameraSpeed = 10.0f * delta_time;
    vec3 right, tmp;

    if ((key == TD_KEY_W || key == TD_KEY_UP) && 
            action == TD_ACTION_PRESS) {
        glm_vec3_scale(cameraFront, cameraSpeed, tmp);
        glm_vec3_add(cameraPos, tmp, cameraPos);
    }
    if ((key == TD_KEY_S || key == TD_KEY_DOWN) && 
            action == TD_ACTION_PRESS) {
        glm_vec3_scale(cameraFront, cameraSpeed, tmp);
        glm_vec3_sub(cameraPos, tmp, cameraPos);
    }
    if ((key == TD_KEY_A || key == TD_KEY_LEFT) && 
            action == TD_ACTION_PRESS) {
        glm_vec3_cross(cameraFront, cameraUp, right);
        glm_vec3_normalize(right);
        glm_vec3_scale(right, cameraSpeed, tmp);
        glm_vec3_sub(cameraPos, tmp, cameraPos);
    }
    if ((key == TD_KEY_D || key == TD_KEY_RIGHT) &&
            action == TD_ACTION_PRESS) {
        glm_vec3_cross(cameraFront, cameraUp, right);
        glm_vec3_normalize(right);
        glm_vec3_scale(right, cameraSpeed, tmp);
        glm_vec3_add(cameraPos, tmp, cameraPos);
    }
}

int main(int argc, char** argv)
{
    float rotate_speed = 1.0f;
    td_bool enable = 1;
    td_ivec2 size = { 0 };
    float out[sizeof(vertices) / sizeof(vertices[0])] = {};
    //td_font *white_font = 0, *red_font = 0;
    td_texture_t *fbtex = 0;

    exu_paramaters_t p = {0};

    p = exu_parse_args(argc, argv, (aparse_arg[1]){
        aparse_arg_option(
                "-rotspd", "--rotation-speed", 
                &rotate_speed, sizeof(rotate_speed), 
                APARSE_ARG_TYPE_FLOAT, "Rotation speed of the RGB cube")
    }, 1, 0);
    if (!td_init())
        return 1;

    //use_params(&p);

    
    //white_font = td_default_font((td_rgba){.r=255, .g=255, .b=255, .a=255}, (td_rgba){0});
    //red_font = td_default_font((td_rgba){.r=255, .a=255}, (td_rgba){0});

    //td_option(TD_OPT_DEPTH_BUFFER, 0, &enable);
    //td_option(TD_OPT_SHIFT_TRANSLATE, 0, &enable);
    td_set_key_callback(key_callback);

    //td_renderer_set_clear_color((td_rgba){.a=255});
    //fbtex = td_get_framebuffer();

    double last_log = get_time();
    const double max_dt = 1.0 / p.max_fps;
    const double program_start = get_time();
    while (td_window_is_running()) {
        double start_frame = get_time();
        double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

        td_poll_events();

        td_renderer_clear();

        td_option(td_opt_display_size, 1, &size);
            
        mat4 model, view, projection, mvp;
        glm_mat4_identity(model);
        glm_mat4_identity(view);
        glm_mat4_identity(projection);
        glm_rotate(model, 
                (td_f32)(get_time() - program_start) * rotate_speed,
                (td_f32[3]){0.5f, 1.0, 0.0f});
        vec3 center;
        glm_vec3_add(cameraPos, cameraFront, center);  // center = cameraPos + cameraFront
        glm_lookat(cameraPos, center, cameraUp, view);
        glm_perspective(glm_rad(45.0f), (td_f32)size.x/size.y, 0.1f, 100.0f, projection);
            
        glm_mat4_mul(projection, view, mvp);
        glm_mat4_mul(mvp, model, mvp);
            
        for (int i = 0; i < sizeof(vertices) / sizeof(vertices[0]) / 3; i++)
        {
            float *dest = &out[i * 3];
            vec4 vertex = {vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2], 1.0f};
            glm_mat4_mulv(mvp, vertex, vertex);  // Multiply by MVP matrix

            td_add_vertex(vertex, (td_vtx_attb[1]){TDVA_POSITION_4D}, 1, TD_FALSE);
            td_add_vertex(vertex_colors + (i % 3) * 3, (td_vtx_attb[1]){TDVA_COLOR_RGB}, 1, TD_TRUE);
        }
        
        char *string = to_string("%.2f", fps);
        td_render_string_into(white_font, (td_ivec2){0}, string, -1, fbtex);

        char *tmp = to_string("%.2f, %.2f, %.2f", cameraPos[0], cameraPos[1], cameraPos[2]);
        // texture = td_render_string(red_font, tmp, -1, &size);
        // td_copy_texture(texture, (td_ivec2){.y=display_size.y - size.y});
        // td_texture_destroy(texture);
        free(tmp);

        td_render();

        while ((delta_time = get_time() - start_frame) < max_dt) {}
        if (start_frame - last_log >= LOG_INTERVAL) {
            write_log("FPS: %s", string);
            last_log = get_time();

        }
        free(string);
    }

    td_destroy_font(white_font);
    td_destroy_font(red_font);

    td_quit();

    stop_logging();
    return 0;
}

