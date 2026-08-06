#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>


#include <time.h>

#include <aparse.h>
#include <stb_image_write.h>
#include <softrast/softrast.h>

#include "log.h"
#include "cpu.h"


static uint64_t get_time_ns(void)
{
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#define NANOSECOND(s) ((uint64_t)((s) * 1000000000ULL))

static void vertex_shader(
        const swrz_vertex_input_t *input,
        swrz_vertex_output_t *output)
{
    (void)output;
    output->position[0] = input->attr[0].vec4[0];
    output->position[1] = input->attr[0].vec4[1];
    output->position[2] = input->attr[0].vec4[2];
    output->position[3] = input->attr[0].vec4[3];
}

static void print_cpu_info(void)
{
    cpu_info_t cpu_info = cpu_query_info();

    info("cpu vendor: %s", cpu_info.vendor);
    info("cpu name: %s", cpu_info.name);
    info("logical/online: %d/%d", cpu_info.core_logical, cpu_info.core_online);
    for(int i = 0; i < cpu_info.cluster_count; i++)
    {
        info("cluster no.%d:", i + 1);
        info("    max frequency: %d MHz", cpu_info.cluster[i].max_mhz);
        info("    affected cores: %d cores", cpu_info.cluster[i].cpu_count);
        for(int j = 0; j < cpu_info.cluster[i].cpu_count; j++)
            info("       core no.%d", cpu_info.cluster[i].cpus[j] + 1);
    }
    info("features:");
    for(int i = 0; i < 32; i++)
    {
        if(!(cpu_info.features & (1U << i)))
            continue;
        info("    %s (%s)", 
                cpu_get_feature_name(i),
                cpu_get_feature_description(i));
    }

}

int main(int argc, char **argv)
{
    static const float vertices[] =
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f
    };

    const char *output = "output.png";
    double duration = 10.0;
    double interval = 0.5;
    aparse_arg main_args[] =
    {
        aparse_arg_option(
                "-o", "--output", 
                &output, 0,
                APARSE_ARG_TYPE_STRING,
                "Path to saved final framebuffer (default: output.png"),
        aparse_arg_option(
                "-d", "--duration",
                &duration, sizeof(duration),
                APARSE_ARG_TYPE_FLOAT,
                "Duration of the benchmark in seconds (default: 10.0s)"),
        aparse_arg_option(
                "-i", "--interval",
                &interval, sizeof(interval),
                APARSE_ARG_TYPE_FLOAT,
                "Log interval in seconds (default: .5s)"),
        aparse_arg_end_marker
    };

    swrz_error_t err = SWRZ_ERR_OK;
    swrz_rasterizer_t *rz = NULL;
    swrz_texture_t *fb = NULL;
    swrz_vertex_array_t vao = {0};

    uint64_t duration_ns = 0;
    uint64_t interval_ns = 0;
    uint64_t last_log = 0;
    uint64_t elapsed = 0;
    uint32_t frame_count = 0;
    uint64_t avg_frametime = 0;
    // struct utsname ustbuf = {0};
    // uname(&ustbuf);

    if(aparse_parse(
            argc, argv, 
            main_args, NULL, 
            NULL) != APARSE_STATUS_OK)
        return 1;
    
    err = swrz_rasterizer_create(&rz, 1920, 1080);
    if(err != SWRZ_ERR_OK)
    {
        error("failed to create rasterizer");
        info("reason: %u", err);
        return 1;
    }

    vao.bindings[0].data = vertices;
    vao.bindings[0].stride = 2 * sizeof(float);

    vao.attributes[0].enabled    = true;
    vao.attributes[0].binding    = 0;
    vao.attributes[0].offset     = 0;
    vao.attributes[0].type       = SWRZ_DATA_F32;
    vao.attributes[0].components = 2;
    vao.attributes[0].normalized = false;
    

    duration_ns = NANOSECOND(duration);
    interval_ns = NANOSECOND(interval);
    last_log = get_time_ns();
    for(;;)
    {
        uint64_t frame_start = 0;
        uint64_t frame_end = 0;

        frame_start = get_time_ns();
        swrz_rasterizer_clear_color(rz, 1.0f, 1.0f, 1.0f, 1.0f);
        swrz_rasterizer_bind_vao(rz, &vao);
        swrz_rasterizer_bind_vertex_shader(rz, vertex_shader);
        swrz_rasterizer_draw_array(
                rz, SWRZ_PRIMITIVE_TRIANGLE, 0, 3);
        frame_end = get_time_ns();

        frame_count++;
        elapsed += frame_end - frame_start;

        if(frame_end - last_log >= interval_ns)
        {
            avg_frametime = elapsed / frame_count;
            debug("frame_count=%u, elapsed=%" PRIu64 ".%09" PRIu64 "s, "
                 "avg_frametime=%" PRIu64 ".%09" PRIu64 "s",
                 frame_count,
                 elapsed / NANOSECOND(1),
                 elapsed % NANOSECOND(1),
                 avg_frametime / NANOSECOND(1),
                 avg_frametime % NANOSECOND(1));

            last_log = frame_end;
        }

        if(elapsed >= duration_ns)
            break;
    }

    debug("saving final framebuffer frame into \"%s\"", output);
    fb = swrz_rasterizer_get_framebuffer(rz);
    stbi_write_png(output, 
            (int)swrz_texture_get_width(fb),
            (int)swrz_texture_get_height(fb),
            4,
            swrz_texture_get_data(fb),
            (int)swrz_texture_get_row_pitch(fb));
    info("saved final frame into \"%s\" successfully", output);
    swrz_rasterizer_destroy(rz);

    print_cpu_info();
    info("performance: %u frames in %" PRIu64 ".%09" PRIu64 "s, "
            "average: %" PRIu64 ".%09" PRIu64 "s",
             frame_count,
             elapsed / NANOSECOND(1),
             elapsed % NANOSECOND(1),
             avg_frametime / NANOSECOND(1),
             avg_frametime % NANOSECOND(1));

    
    return 0;
}
