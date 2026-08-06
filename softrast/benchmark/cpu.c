#include "cpu.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/auxv.h>

#include <sys/system_properties.h>
#include <sys/sysinfo.h>

#include "log.h"
#include "str.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ARRSZ(arr) (sizeof((arr)) / sizeof((arr)[0]))

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

static void cpu__qualcomm_model_to_name(
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

static void cpu__mediatek_model_to_name(
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

static void cpu__query_name(
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
        cpu__qualcomm_model_to_name(prop_cpu, 
                cpu->name, sizeof(cpu->name)); 
        return;
    }
    
    if(str_compare_ignore(prop_vendor, "MediaTek")) // also Mediatek
    {
        __system_property_get("ro.mediatek.platform", prop_cpu);
        if(!str_starts_with(prop_cpu, "MT"))
            warn("unhandled mediatek cpu model: \"%s\"", prop_cpu);
        str_copy(cpu->vendor, sizeof(cpu->vendor), "MediaTek");
        cpu__mediatek_model_to_name(prop_cpu, 
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

static void cpu__parse_cpuinfo(cpu_info_t *cpu_info)
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

static void cpu__query_frequency(cpu_info_t *cpu_info)
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


static void cpu__query_feature(
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

cpu_info_t cpu_query_info(void)
{
    cpu_info_t cpu_info = {0};
    cpu__query_name(&cpu_info);
    cpu__parse_cpuinfo(&cpu_info);
    cpu__query_frequency(&cpu_info);
    cpu__query_feature(&cpu_info);
    return cpu_info;
}

const char *cpu_get_feature_name(
        const int bit_idx) 
{
    const char *feature_names[32] =
    {
        [0] = "fphp",   
        [1] = "fp",     

        [2] = "asimdhp",
        [3] = "asimdfhm",
        [4] = "asimd",  
        [5] = "asimddp",

        [6] = "sve",    
        [7] = "sve2",   

        [8] = "atomics",
    };
    
    if(bit_idx < 0 || bit_idx >= 32)
        return "unknown";
    return feature_names[bit_idx] ? 
        feature_names[bit_idx] : "unknown";
}

const char *cpu_get_feature_description(
        const int bit_idx) 
{
    const char *feature_descriptions[32] =
    {
        [0] = "ARM Half-precision floating point",
        [1] = "ARM Hardware floating-point",

        [2] = "ARM Half-precision NEON",
        [3] = "ARM NEON fused half-precision multiply-add",
        [4] = "ARM Advanced SIMD (NEON)",
        [5] = "ARM NEON dot-product",

        [6] = "ARM Scalable Vector Extension",
        [7] = "ARM Scalable Vector Extension 2",

        [8] = "ARM Hardware atomics (LSE atomics)"
    };
    
    if(bit_idx < 0 || bit_idx >= 32)
        return "unknown";
    return feature_descriptions[bit_idx] ? 
        feature_descriptions[bit_idx] : "unknown";
}

