#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

#include <time.h>

#include <aparse.h>
#include <lodepng.h>
#include <softrast/softrast.h>

#include "log.h"
#include "cpu.h"
#include "sha256.h"

#define WIDTH  1920
#define HEIGHT 1080

static uint64_t get_time_ns(void)
{
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static int g_malloc_count = 0;
static int g_free_count = 0;

static size_t g_malloc_bytes = 0;
static size_t g_free_bytes = 0;

static void *custom_malloc(
        void *userdata,
        size_t size)
{
    size_t *h = NULL;
    void *p = NULL;

    (void)userdata;

    g_malloc_count++;
    g_malloc_bytes += size;

    h = (size_t *)malloc(sizeof(size_t) + size);
    if (h == NULL)
        return NULL;

    h[0] = size;
    p = (void *)(h + 1);
    return p;
}

static void *custom_realloc(
        void *userdata,
        void *p, 
        size_t size)
{
    size_t *h = NULL;
    size_t *new_h = NULL;
    void *new_p = NULL;
    size_t old_size = 0;
    
    (void)userdata;

    if (p == NULL)
        return custom_malloc(NULL, size);

    h = ((size_t *)p) - 1;
    old_size = h[0];

    new_h = realloc(h, sizeof(size_t) + size);
    if (new_h == NULL)
        return NULL;

    if (size > old_size)
        g_malloc_bytes += (size - old_size);
    else
        g_free_bytes += (old_size - size);

    new_h[0] = size;
    new_p = (void *)(new_h + 1);
    return new_p;
}

static void custom_free(
        void *userdata,
        void *p)
{
    size_t *h = NULL;

    (void)userdata;
    if (p == NULL)
        return;

    g_free_count++;
    h = ((size_t *)p) - 1;
    g_free_bytes += h[0];

    free(h);
}

static void print_memory_stat(void)
{
    info("memory stats:");
    info("    malloc: %d times, %zu bytes",
            g_malloc_count, g_malloc_bytes);
    info("    free: %d times, %zu bytes",
            g_free_count, g_free_bytes);
    info("    live: %zu bytes",
            (g_malloc_bytes > g_free_bytes)
               ? (g_malloc_bytes - g_free_bytes)
               : 0);
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

static void hash_cpu_info(
        const cpu_info_t *cpu_info,
        char hash[65])
{
    char canonical[4096] = {0};
    int len = 0;
    if(!cpu_info || !hash)
        return;

    len = snprintf(
        canonical,
        sizeof(canonical),
        "name=%s\n"
        "arch=%s\n"
        "l1=%u\n"
        "l2=%u\n"
        "l3=%u\n"
        "logical=%d\n"
        "online=%d\n"
        "clusters=%d\n"
        "features=%08x\n",
        cpu_info->name,
        cpu_info->arch,
        cpu_info->l1_size,
        cpu_info->l2_size,
        cpu_info->l3_size,
        cpu_info->core_logical,
        cpu_info->core_online,
        cpu_info->cluster_count,
        cpu_info->features);

    if(len < 0 || (size_t)len >= sizeof(canonical))
        return;

    for(int i = 0; i < cpu_info->cluster_count; i++)
    {
        int n = snprintf(
            canonical + len,
            sizeof(canonical) - (size_t)len,
            "cluster=%d,%d,%d",
            i,
            cpu_info->cluster[i].max_mhz,
            cpu_info->cluster[i].cpu_count);

        if(n < 0 || (size_t)n >= sizeof(canonical) - (size_t)len)
            return;

        len += n;
        for(int j = 0; j < cpu_info->cluster[i].cpu_count; j++)
        {
            n = snprintf(
                canonical + len,
                sizeof(canonical) - (size_t)len,
                ",%d",
                cpu_info->cluster[i].cpus[j]);

            if(n < 0 || (size_t)n >= sizeof(canonical) - (size_t)len)
                return;

            len += n;
        }

        if((size_t)len + 1 >= sizeof(canonical))
            return;

        canonical[len++] = '\n';
        canonical[len] = '\0';
    }

    sha256_easy_hash_hex(
        canonical,
        (size_t)len,
        hash);
}

static void print_cpu_info(
        const cpu_info_t *cpu_info)
{
    char hash[65] = {0};

    info("cpu name: \"%s\"", cpu_info->name);
    info("arch: \"%s\"", cpu_info->arch);
    info("cache size: ");
    info("    L1 size: %u KB", cpu_info->l1_size / 1024);
    info("    L2 size: %u KB", cpu_info->l2_size / 1024);
    info("    L3 size: %u KB", cpu_info->l3_size / 1024);

    info("logical/online: %d/%d", cpu_info->core_logical, cpu_info->core_online);
    for(int i = 0; i < cpu_info->cluster_count; i++)
    {
        info("cluster no.%d:", i + 1);
        info("    max frequency: %d MHz", cpu_info->cluster[i].max_mhz);
        info("    affected cores: %d cores", cpu_info->cluster[i].cpu_count);
        for(int j = 0; j < cpu_info->cluster[i].cpu_count; j++)
            info("       core no.%d", cpu_info->cluster[i].cpus[j] + 1);
    }
    info("features:");
    for(int i = 0; i < 32; i++)
    {
        if(!(cpu_info->features & (1U << i)))
            continue;
        info("    %s (%s)", 
                cpu_get_feature_name(i),
                cpu_get_feature_description(i));
    }

    sha256_easy_hash_hex(cpu_info, sizeof(*cpu_info), hash);
    info("cpu info hash: \"%s\"", hash);
}

static uint8_t *pack_rgba8(
        const uint8_t *data,
        size_t row_pitch)
{
    size_t row_size = 0;
    size_t size = 0;
    uint8_t *packed = NULL;

    if(!data)
        return NULL;

    row_size = (size_t)WIDTH * 4;
    if(row_pitch < row_size)
        return NULL;

    size = row_size * HEIGHT;
    packed = malloc(size);
    if(!packed)
        return NULL;

    for(uint32_t y = 0; y < HEIGHT; y++)
    {
        memcpy(
            packed + (size_t)y * row_size,
            data + (size_t)y * row_pitch,
            row_size);
    }

    return packed;
}

#define CHECK(x, err, label) \
    if(((err) = x) != 0) goto label

static unsigned add_png_text_uint(
        LodePNGInfo *png_info,
        const char *key,
        unsigned value)
{
    char value_str[32] = {0};
    snprintf(value_str, sizeof(value_str), "%u", value);
    return lodepng_add_text(png_info, key, value_str);
}

static void encode_png_rgba(
        const char *path,
        const uint8_t *data,
        const cpu_info_t *cpu_info)
{
    unsigned err = 0;
    LodePNGState state = {0};
    uint8_t *png = NULL;
    size_t png_size = 0;
    char key[64] = {0};
    char value[256] = {0};

    if(!path || !data || !cpu_info)
        return;

    lodepng_state_init(&state);

    CHECK(lodepng_add_text(&state.info_png, "swrz_type",
                "benchmark"), err, cleanup);
    CHECK(lodepng_add_text(&state.info_png, "swrz_cpu_name",
                cpu_info->name), err, cleanup);
    CHECK(lodepng_add_text(&state.info_png,"swrz_cpu_arch",
                cpu_info->arch), err, cleanup);
    CHECK(add_png_text_uint(&state.info_png, "swrz_cpu_l1_kb",
                cpu_info->l1_size / 1024), err, cleanup);
    CHECK(add_png_text_uint(&state.info_png, "swrz_cpu_l2_kb",
                cpu_info->l2_size / 1024), err, cleanup);
    CHECK(add_png_text_uint(&state.info_png, "swrz_cpu_l3_kb",
                cpu_info->l3_size / 1024), err, cleanup);
    CHECK(add_png_text_uint(&state.info_png, "swrz_cpu_logical",
                cpu_info->core_logical), err, cleanup);
    CHECK(add_png_text_uint(&state.info_png, "swrz_cpu_online",
                cpu_info->core_online), err, cleanup);
    for(int i = 0; i < cpu_info->cluster_count; i++)
    {
        snprintf(key, sizeof(key),
                "swrz_cpu_cluster_%d_max_mhz", i);
        CHECK(add_png_text_uint(&state.info_png, key,
                    cpu_info->cluster[i].max_mhz), err, cleanup);
        snprintf(key, sizeof(key),
            "swrz_cpu_cluster_%d_cpus", i);

        value[0] = '\0';
        for(int j = 0; j < cpu_info->cluster[i].cpu_count; j++)
        {
            size_t used = strlen(value);
            snprintf(value + used, sizeof(value) - used,
                "%s%u",
                j ? "," : "",
                cpu_info->cluster[i].cpus[j]);
        }

        CHECK(lodepng_add_text(&state.info_png, key,
                    value), err, cleanup);
    }

    value[0] = '\0';
    for(int i = 0; i < 32; i++)
    {
        if(!(cpu_info->features & (1U << i)))
            continue;

        if(value[0] != '\0')
            strncat(value, ",", sizeof(value) - strlen(value) - 1);

        strncat(value, cpu_get_feature_name(i),
            sizeof(value) - strlen(value) - 1);
    }

    CHECK(lodepng_add_text(&state.info_png, "swrz_cpu_features",
                value), err, cleanup);

    {
        char hash[65] = {0};
        hash_cpu_info(cpu_info, hash);
        CHECK(lodepng_add_text(&state.info_png, "swrz_cpu_info_hash",
                    hash), err, cleanup);
    }

    state.info_raw.colortype = LCT_RGBA;
    state.info_raw.bitdepth = 8;

    CHECK(lodepng_encode(
                &png,
                &png_size,
                data,
                WIDTH,
                HEIGHT,
                &state),
            err, cleanup);

    CHECK(lodepng_save_file(
                png,
                png_size,
                path),
            err, cleanup);

cleanup:
    if(err != 0)
    {
        error("failed to encode png to \"%s\"", path);
        info("reason: %s", lodepng_error_text(err));
    }

    free(png);
    lodepng_state_cleanup(&state);
}
int main(int argc, char **argv)
{
    static const float vertices[] =
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f
    };

    bool spec_only = false;
    bool no_output = false;
    const char *output = "output.png";
    double duration = 10.0;
    double interval = 0.5;
    aparse_arg main_args[] =
    {
        aparse_arg_option(
                "-so", "--spec-only", 
                &spec_only, sizeof(spec_only),
                APARSE_ARG_TYPE_BOOL,
                "Only export machine specs (default: false)"),
        aparse_arg_option(
                "-no", "--no-output", 
                &no_output, sizeof(no_output),
                APARSE_ARG_TYPE_BOOL,
                "Disable saving final framebuffer frame (default: false)"),
        aparse_arg_option(
                "-o", "--output", 
                &output, 0,
                APARSE_ARG_TYPE_STRING,
                "Path to saved final framebuffer frame (default: output.png"),
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

    cpu_info_t cpu_info = {0};

    if(aparse_parse(
            argc, argv, 
            main_args, NULL, 
            NULL) != APARSE_STATUS_OK)
        return 1;

    if(spec_only)
    {
        cpu_info = cpu_query_info();
        print_cpu_info(&cpu_info);
        return 0;
    }

    swrz_set_allocator(&(swrz_allocator_t){
            .malloc = custom_malloc,
            .realloc = custom_realloc,
            .free = custom_free
    });
    
    err = swrz_rasterizer_create(&rz, WIDTH, HEIGHT);
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

    cpu_info = cpu_query_info();
    if(!no_output)
    {
        uint8_t *packed_fb = NULL;
        debug("saving final framebuffer frame into \"%s\"", output);
        fb = swrz_rasterizer_get_framebuffer(rz);
        packed_fb = pack_rgba8(
                swrz_texture_get_data(fb),
                swrz_texture_get_row_pitch(fb));
        if(!packed_fb)
            error("failed to pack framebuffer");
        else
        {
            encode_png_rgba(output, packed_fb, &cpu_info);
            free(packed_fb);
            info("saved final frame into \"%s\" successfully", output);
        }
    }
    swrz_rasterizer_destroy(rz);

    info("performance: %u frames in %" PRIu64 ".%09" PRIu64 "s, "
            "average: %" PRIu64 ".%09" PRIu64 "s",
             frame_count,
             elapsed / NANOSECOND(1),
             elapsed % NANOSECOND(1),
             avg_frametime / NANOSECOND(1),
             avg_frametime % NANOSECOND(1));
    print_memory_stat();

    
    return 0;
}
