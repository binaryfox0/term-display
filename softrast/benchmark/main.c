#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/auxv.h>

#include <sys/system_properties.h>
#include <sys/sysinfo.h>

#include <aparse.h>
#include <stb_image_write.h>
#include <softrast/softrast.h>

#define debug aparse_prog_debug
#define info aparse_prog_info
#define warn aparse_prog_warn
#define error aparse_prog_error

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAX_CPU_NAME_LENGTH 128
#define MAX_CPU_VENDOR_LENGTH 32
#define MAX_CLUSTER_COUNT 8
#define MAX_CPU_PER_CLUSTER_COUNT 32

#define ARRSZ(arr) (sizeof((arr)) / sizeof((arr)[0]))

typedef struct
{
    char name[MAX_CPU_NAME_LENGTH];
    char vendor[MAX_CPU_VENDOR_LENGTH];

    uint16_t core_logical;
    uint16_t core_online;
    uint32_t features;

    struct
    {
        uint16_t max_mhz;
        uint16_t cpus[MAX_CPU_PER_CLUSTER_COUNT];
        int cpu_count;
    } cluster[MAX_CLUSTER_COUNT];
    int cluster_count;
    
} cpu_info_t;


static void str_copy(
        char *dst, 
        const size_t dst_size, 
        const char *src)
{
    size_t i = 0;

    if (dst == NULL || src == NULL || dst_size == 0) {
        return;
    }

    while ((i < (dst_size - 1)) && (src[i] != '\0')) {
        dst[i] = src[i];
        ++i;
    }

    dst[i] = '\0';
}

static bool str_starts_with(
        const char *str, 
        const char *prefix)
{
    while (*prefix != '\0')
    {
        if (*str == '\0' || *str != *prefix)
            return false;

        str++;
        prefix++;
    }

    return true;
}

static bool str_empty(const char *str) {
    return str[0] == '\0';
}

static inline bool str_compare(const char *a, const char *b) {
    return strcmp((a), (b)) == 0;
}

static bool str_compare_ignore(const char *a, const char *b)
{
    unsigned char ca = 0;
    unsigned char cb = 0;

    while (*a != '\0' && *b != '\0')
    {
        ca = (unsigned char)tolower((unsigned char)*a);
        cb = (unsigned char)tolower((unsigned char)*b);

        if (ca != cb)
            return false;

        ++a;
        ++b;
    }

    return (*a == '\0' && *b == '\0');
}

static bool get_android_cpu_vendor(
        char *prop_value)
{
    if(__system_property_get("ro.soc.manufacturer", prop_value) > 0)
    {
        debug("ro.soc.manufacturer: \"%s\"", prop_value);
        return true;
    }

    if(__system_property_get(
                "ro.product.product.manufacturer", prop_value) > 0)
    {
        debug("ro.product.product.manufacturer: \"%s\"", prop_value);
        return true;
    }

    if(__system_property_get(
                "ro.product.vendor.manufacturer", prop_value) > 0)
    {
        debug("ro.product.vendor.manufacturer: \"%s\"", prop_value);
        return true;
    }

    if(__system_property_find("ro.mediatek.platform"))
    {
        str_copy(prop_value, PROP_VALUE_MAX, "MediaTek");
        return true;
    }

    error("failed to get android cpu vendor");
    return false;
}

static void qualcomm_cpu_to_name(
        const char *prop_value,
        char *buf,
        const size_t size) 
{
    uint32_t code = 0;
    const char *name = NULL;

    assert(strlen(prop_value) > 2);
    code = (uint32_t)strtoul(prop_value + 2, NULL, 10);

    // https://en.wikipedia.org/wiki/List_of_Qualcomm_Snapdragon_systems_on_chips
    switch (code) 
    {
        case 8845: name = "8 Gen 5";       break; // ? 
        case 8850: name = "8 Elite Gen 5"; break; 
        case 8735: name = "8s Gen 4";      break;
        case 8750: name = "8 Elite";       break;
        case 8635: name = "8s Gen 3";      break;
        case 8650: name = "8 Gen 3";       break;
        case 8550: name = "8 Gen 2";       break;
        case 8475: name = "8+ Gen 1";      break;
        case 8450: name = "8 Gen 1";       break;
        case 7750: name = "7 Gen 4";       break;
        case 7675: name = "7+ Gen 3";      break;
        case 7635: name = "7s Gen 3";      break;
        case 7550: name = "7 Gen 3";       break;
        case 7475: name = "7+ Gen 2";      break;
        case 7435: name = "7s Gen 2";      break;
        case 7450: name = "7 Gen 1";       break;
        case 6650: name = "6 Gen 4";       break;
        case 6375: name = "6s Gen 3";      break;
        case 6475: name = "6 Gen 3";       break;
        case 6115: name = "6s Gen 1";      break;
        case 6450: name = "6 Gen 1";       break;
        case 4635: name = "4s Gen 2";      break;
        case 4450: name = "4 Gen 2";       break;
        case 4375: name = "4 Gen 1";       break; 
        default: break;
    }

    if (name) 
        snprintf(buf, size, "Qualcomm Snapdragon %s", name);
    else
        str_copy(buf, size, prop_value);
}

static void mediatek_cpu_to_name(
        const char *prop_value,
        char *buf,
        const size_t size) 
{
    uint32_t code = 0;
    const char *name = NULL;

    assert(strlen(prop_value) > 2);
    code = (uint32_t)strtoul(prop_value + 2, NULL, 10);

    // https://en.wikipedia.org/wiki/List_of_MediaTek_systems_on_chips
    switch (code)
    {
        case 6993: name = "9500"; break;
        case 6991: name = "9400"; break;
        case 6989:
        case 8796: name = "9300"; break;
        case 6985: name = "9200"; break;
        case 6983:
        case 8798: name = "9000"; break;

        case 6899: name = "8400"; break;
        case 6897:
        case 8792: name = "8300"; break;
        case 6896: name = "8200"; break;
        case 8795: name = "8100"; break;
        case 6895: name = "8000"; break;

        default: break;
    }

    if (name) 
        snprintf(buf, size, "MediaTek Dimensity %s", name);
    else
        str_copy(buf, size, prop_value);
}

static void exynos_cpu_to_name(
        const char *prop_value,
        char *buf,
        const size_t size) 
{

    uint32_t code = 0;
    const char *name = NULL;

    assert(strlen(prop_value) > 3);
    code = (uint32_t)strtoul(prop_value + 3, NULL, 10);

    // https://en.wikipedia.org/wiki/Exynos
    switch (code) 
    {
        case 9965: name = "2600"; break;
        case 9955: name = "2500"; break;
        case 9945: name = "2400"; break;
        // No 2300
        case 9925: name = "2200"; break;
        case 9840: name = "2100"; break;

        case 8855: name = "1580"; break;
        case 8845: name = "1480"; break;
        case 8835: name = "1380"; break;
        case 8535: name = "1330"; break;
        case 8825: name = "1280"; break;
        case 9815: name = "1080"; break;

        case 9830: name = "990"; break;
        case 9630: name = "980"; break;

        case 8805: name = "880"; break;
        case 3830: name = "850"; break;

        default: break;
    }
    
    if (name) 
        snprintf(buf, size, "Samsung Exynos %s", name);
    else
        str_copy(buf, size, prop_value);
}

static void get_android_cpu(
        cpu_info_t *cpu)
{
    char prop_vendor[PROP_VALUE_MAX] = {0};
    char prop_cpu[PROP_VALUE_MAX] = {0};
    if(__system_property_get("ro.cpu.model", prop_cpu) > 0)
        str_copy(cpu->name, sizeof(cpu->name), prop_cpu); 
    
    get_android_cpu_vendor(prop_vendor);
    if(str_compare(prop_vendor, "QTI"))
    {
        if(!str_starts_with(prop_cpu, "SM"))
            warn("unhandled qualcomm cpu model: \"%s\"", prop_cpu);
        str_copy(cpu->vendor, sizeof(cpu->vendor), "Qualcomm");
        qualcomm_cpu_to_name(prop_cpu, 
                cpu->name, sizeof(cpu->name)); 
        return;
    }
    
    if(str_compare_ignore(prop_vendor, "MediaTek")) // also Mediatek
    {
        __system_property_get("ro.mediatek.platform", prop_cpu);
        if(!str_starts_with(prop_cpu, "MT"))
            warn("unhandled mediatek cpu model: \"%s\"", prop_cpu);
        str_copy(cpu->vendor, sizeof(cpu->vendor), "MediaTek");
        mediatek_cpu_to_name(prop_cpu, 
                cpu->name, sizeof(cpu->name)); 
        return;
    }

    if(str_compare(prop_vendor, "Samsung"))
    {
        if(!str_starts_with(prop_cpu, "s5e"))
            warn("unhandled exynos cpu model: \"%s\"", prop_cpu);
        str_copy(cpu->vendor, sizeof(cpu->vendor), "Samsung");

        // cosmetic
        cpu->name[0] = 'S';
        cpu->name[2] = 'E';
        exynos_cpu_to_name(prop_cpu, 
                cpu->name, sizeof(cpu->name)); 
        return;
    }

    warn("unhandled android cpu vendor: \"%s\"", prop_vendor);
}

static void parse_cpuinfo(cpu_info_t *cpu_info)
{
    FILE *file = NULL;
    char line[256] = {0};

    file = fopen("/proc/cpuinfo", "r");
    if(!file)
    {
        error("failed to open /proc/cpuinfo");
        return;
    }

    while(fgets(line, sizeof(line), file))
    {
        char *colon = NULL;
        char *key = NULL;
        char *value = NULL;

        colon = strchr(line, ':');
        if(!colon)
            continue;

        key = line;
        value = colon + 1;
        while(*value == ' ' || *value == '\t')
            value++;
        value[strcspn(value, "\r\n")] = '\0';

        while(colon > key + 1 && !isalpha(*(colon - 1)))
            colon--;
        *colon = '\0';
        
        if(str_compare(key, "Hardware") && 
                str_empty(cpu_info->name))
            str_copy(cpu_info->name, sizeof(cpu_info->name), 
                    value);
        
    }

    fclose(file);
}

static inline bool read_file_fixed(
        int fd,
        char *buf,
        const size_t size)
{
    struct stat st = {0};
    size_t read_size = 0;
    size_t total_read = 0;

    if(fd < 0 || !buf || size == 0)
        return false;

    if(fstat(fd, &st) < 0)
        return false;

    read_size = MIN(size - 1, (size_t) st.st_size);
    while(total_read < read_size)
    {
        ssize_t success = read(
                fd,
                buf + total_read,
                read_size - total_read);

        if(success < 0)
            return false;

        if(success == 0)
            break;

        total_read += (size_t) success;
    }

    buf[total_read] = '\0';
    return true;
}

static inline bool read_file_fixed_rel(
        int dir_fd,
        const char *name,
        char *buf,
        const size_t size)
{
    int fd = -1;
    if(dir_fd < 0 || !name || !buf || size == 0)
        return false;
    
    fd = openat(dir_fd, name, O_RDONLY);
    if(fd < 0)
        return false;
    read_file_fixed(fd, buf, size);
    close(fd);
    return true;
}

static void get_cpu_frequency(cpu_info_t *cpu_info)
{
    DIR *dir = NULL;
    int dir_fd = -1;
    struct dirent *entry = NULL;

    dir = opendir("/sys/devices/system/cpu/cpufreq/");
    if(!dir)
    {
        error("failed to get cpu frequency");
        return;
    }

    dir_fd = dirfd(dir);

    while((entry = readdir(dir)))
    {
        int policy_fd = -1;
        int cluster_idx = 0;
        int cpu_index = 0;
        uint16_t cpu = 0;
        char buffer[1024] = {0};
        char *token = NULL;

        if(!str_starts_with(entry->d_name, "policy") ||
           !isdigit(entry->d_name[sizeof("policy") - 1]))
            continue;

        cluster_idx = cpu_info->cluster_count;
        if(cluster_idx >= (int)ARRSZ(cpu_info->cluster))
        {
            error("too much cluster");
            break;
        }

        policy_fd = openat(dir_fd, entry->d_name, 
                O_DIRECTORY | O_PATH);
        if(policy_fd < 0)
        {
            error("failed to open policy directory");
            continue;
        }

        if(!read_file_fixed_rel(policy_fd, "scaling_max_freq",
                                buffer, sizeof(buffer)))
        {
            error("failed to read scaling_max_freq");
            goto next;
        }

        cpu_info->cluster[cluster_idx].max_mhz = 
            (uint16_t)(strtoul(buffer, NULL, 10) / 1000);
        if(!read_file_fixed_rel(policy_fd, "affected_cpus",
                                buffer, sizeof(buffer)))
        {
            error("failed to read affected_cpus");
            goto next;
        }

        token = strtok(buffer, " \n");
        while(token)
        {
            cpu_index = cpu_info->cluster[cluster_idx].cpu_count;
            if(cpu_index >= (int)ARRSZ(cpu_info->cluster[cluster_idx].cpus))
            {
                error("too much cpu per cluster");
                goto next;
            }

            cpu = (uint16_t)strtoul(token, NULL, 10);
            cpu_info->cluster[cluster_idx].cpus[cpu_index] = cpu;
            cpu_info->cluster[cluster_idx].cpu_count++;

            token = strtok(NULL, " \n");
        }

        cpu_info->cluster_count++;
next:   close(policy_fd);
    }

    closedir(dir);
}

enum
{
    CPU_FEATURE_ARM_FPHP        = 1 << 0,
    CPU_FEATURE_ARM_FP          = 1 << 1,

    CPU_FEATURE_ARM_ASIMDHP     = 1 << 2,
    CPU_FEATURE_ARM_ASIMDFHM    = 1 << 3,
    CPU_FEATURE_ARM_ASIMD       = 1 << 4,
    CPU_FEATURE_ARM_ASIMDDP     = 1 << 5,

    CPU_FEATURE_ARM_SVE         = 1 << 6,
    CPU_FEATURE_ARM_SVE2        = 1 << 7,

    CPU_FEATURE_ARM_ATOMICS     = 1 << 8,
};

const char *cpu_feature_readable[32][2] =
{
    [0] = {"fphp",       "ARM Half-precision floating point"},
    [1] = {"fp",         "ARM Hardware floating-point"},

    [2] = {"asimdhp",    "ARM Half-precision NEON"},
    [3] = {"asimdfhm",   "ARM NEON fused half-precision multiply-add"},
    [4] = {"asimd",      "ARM Advanced SIMD (NEON)"},
    [5] = {"asimddp",    "ARM NEON dot-product"},

    [6] = {"sve",        "ARM Scalable Vector Extension"},
    [7] = {"sve2",       "ARM Scalable Vector Extension 2"},

    [8] = {"atomics",    "ARM Hardware atomics (LSE atomics)"},
};

static void query_cpu_feature(
        cpu_info_t *cpu_info)
{
    unsigned long hwcap = 0, hwcap2 = 0;
    uint32_t features = 0;

    hwcap = getauxval(AT_HWCAP);
    hwcap2 = getauxval(AT_HWCAP2);

    features |= hwcap   & HWCAP_FPHP        ? CPU_FEATURE_ARM_FPHP      : 0;
    features |= hwcap   & HWCAP_FP          ? CPU_FEATURE_ARM_FP        : 0;

    features |= hwcap   & HWCAP_ASIMDHP     ? CPU_FEATURE_ARM_ASIMDHP   : 0; 
    features |= hwcap   & HWCAP_ASIMDFHM    ? CPU_FEATURE_ARM_ASIMDFHM  : 0;
    features |= hwcap   & HWCAP_ASIMD       ? CPU_FEATURE_ARM_ASIMD     : 0; 
    features |= hwcap   & HWCAP_ASIMDDP     ? CPU_FEATURE_ARM_ASIMDDP   : 0; 

    features |= hwcap   & HWCAP_SVE         ? CPU_FEATURE_ARM_SVE       : 0;
    features |= hwcap2  & HWCAP2_SVE2       ? CPU_FEATURE_ARM_SVE2      : 0;

    features |= hwcap   & HWCAP_ATOMICS     ? CPU_FEATURE_ARM_ATOMICS   : 0;

    cpu_info->features = features;
}

static uint64_t get_time_ns(void)
{
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#define NANOSECOND(s) ((uint64_t)((s) * 1000000000ULL))

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
        aparse_arg_end_marker
    };

    cpu_info_t cpu_info = {0};

    swrz_error_t err = SWRZ_ERR_OK;
    swrz_rasterizer_t *rz = NULL;
    swrz_texture_t *fb = NULL;
    swrz_vertex_array_t vao = {0};

    uint64_t duration_ns = 0;
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
    
    cpu_info.core_logical = (uint16_t)get_nprocs_conf();
    cpu_info.core_online =  (uint16_t)get_nprocs();

    get_android_cpu(&cpu_info);
    parse_cpuinfo(&cpu_info);
    get_cpu_frequency(&cpu_info);
    query_cpu_feature(&cpu_info);

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
                cpu_feature_readable[i][0],
                cpu_feature_readable[i][1]);
    }

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
    

    duration_ns = NANOSECOND(duration);
    last_log = get_time_ns();
    for(;;)
    {
        uint64_t frame_start = 0;
        uint64_t frame_end = 0;

        frame_start = get_time_ns();
        swrz_rasterizer_clear_color(rz, 1.0f, 1.0f, 1.0f, 1.0f);
        frame_end = get_time_ns();

        frame_count++;
        elapsed += frame_end - frame_start;

        if(frame_end - last_log >= NANOSECOND(0.5))
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
    swrz_rasterizer_destroy(rz);

    info("summary: %u frames in %" PRIu64 ".%09" PRIu64 "s, "
            "average: %" PRIu64 ".%09" PRIu64 "s",
             frame_count,
             elapsed / NANOSECOND(1),
             elapsed % NANOSECOND(1),
             avg_frametime / NANOSECOND(1),
             avg_frametime % NANOSECOND(1));

    
    return 0;
}
