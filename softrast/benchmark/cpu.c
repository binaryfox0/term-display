#include "cpu.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include <dirent.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/auxv.h>

#ifdef __ANDROID__
#   include <sys/system_properties.h>
#endif
#include <sys/sysinfo.h>

#if defined(__x86_64__) || defined(__i386__)
#   include <cpuid.h>
#   include <immintrin.h>
#endif

#include "log.h"
#include "str.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ARRSZ(arr) (sizeof((arr)) / sizeof((arr)[0]))

static inline bool read_file_fixed(
        int fd,
        char *buf,
        const size_t size)
{
    size_t total_read = 0;

    if(fd < 0 || !buf || size == 0)
        return false;

    while(total_read < size - 1)
    {
        ssize_t success = 0;

        success = read(
                fd,
                buf + total_read,
                size - 1 - total_read);

        if(success < 0)
            return false;

        if(success == 0)
            break;

        total_read += (size_t)success;
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
    bool ret = false;

    if(dir_fd < 0 || !name || !buf || size == 0)
        return false;

    fd = openat(dir_fd, name, O_RDONLY);
    if(fd < 0)
        return false;

    ret = read_file_fixed(fd, buf, size);

    close(fd);
    return ret;
}

static bool cpu__get_cpu_name_cpuinfo(
        cpu_info_t *cpu_info)
{
    FILE *file = NULL;
    char line[256] = {0};

    file = fopen("/proc/cpuinfo", "r");
    if(!file)
    {
        error("failed to open /proc/cpuinfo");
        return false;
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
        
        if(
                str_compare(key, "Hardware") ||
                str_compare(key, "model name"))
        {
            str_copy(
                    cpu_info->name, 
                    sizeof(cpu_info->name), 
                    value);
            return true;
        }
    }

    fclose(file);
}


#ifdef __ANDROID__
static void cpu__query_name(
        cpu_info_t *cpu_info)
{
    char prop[PROP_VALUE_MAX] = {0};

    if(__system_property_get("ro.soc.model", prop) > 0)
    {
        str_copy(
                cpu_info->name,
                sizeof(cpu_info->name),
                prop);
        return;
    }

    if(__system_property_get("ro.mediatek.platform", prop) > 0)
    {
        str_copy(
                cpu_info->name,
                sizeof(cpu_info->name),
                prop);
        return;
    }
}
#else
static void cpu__query_name(
        cpu_info_t *cpu_info)
{
    (void)cpu_info;
}
#endif


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

static void cpu__query_cores(
        cpu_info_t *cpu_info)
{
    cpu_info->core_logical = (uint16_t)sysconf(_SC_NPROCESSORS_CONF);
    cpu_info->core_online = (uint16_t)sysconf(_SC_NPROCESSORS_ONLN);
}

static void cpu__query_feature(
        cpu_info_t *cpu_info)
{
    uint32_t features = 0;

#if defined(__aarch64__) || defined(__arm__)
    {
        unsigned long hwcap = 0, hwcap2 = 0;
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
    }
#elif defined(__x86_64__) || defined(__i386__)
    {
    uint32_t eax = 0;
    uint32_t ebx = 0;
    uint32_t ecx = 0;
    uint32_t edx = 0;

    unsigned int max_leaf = 0;
    uint64_t xcr0 = 0;

    if (__get_cpuid(0, &eax, &ebx, &ecx, &edx))
        max_leaf = eax;

    /*
     * CPUID leaf 1:
     *
     * SSE2   : EDX[26]
     * SSSE3  : ECX[9]
     * SSE4.1 : ECX[19]
     * POPCNT : ECX[23]
     * FMA    : ECX[12]
     * F16C   : ECX[29]
     * AVX    : ECX[28]
     * OSXSAVE: ECX[27]
     */
    if (max_leaf >= 1) {
        __cpuid(1, eax, ebx, ecx, edx);

        features |= edx & bit_SSE2
            ? CPU_FEATURE_X86_SSE2 : 0;

        features |= ecx & bit_SSSE3
            ? CPU_FEATURE_X86_SSSE3 : 0;

        features |= ecx & bit_SSE4_1
            ? CPU_FEATURE_X86_SSE41 : 0;

        features |= ecx & bit_POPCNT
            ? CPU_FEATURE_X86_POPCNT : 0;

        /*
         * AVX requires:
         *
         * 1. CPU AVX support
         * 2. OSXSAVE
         * 3. OS enabled XMM/YMM state
         */
        if ((ecx & bit_AVX) &&
            (ecx & bit_OSXSAVE)) {

            xcr0 = _xgetbv(0);

            if ((xcr0 & 0x6) == 0x6) {
                features |= CPU_FEATURE_X86_AVX;

                /*
                 * FMA and F16C use the AVX register state,
                 * so only expose them when AVX is usable.
                 */
                features |= ecx & bit_FMA
                    ? CPU_FEATURE_X86_FMA : 0;

                features |= ecx & bit_F16C
                    ? CPU_FEATURE_X86_F16C : 0;
            }
        }
    }

    if (max_leaf >= 7) {
        __cpuid_count(7, 0, eax, ebx, ecx, edx);

        features |= ebx & bit_BMI
            ? CPU_FEATURE_X86_BMI1 : 0;

        features |= ebx & bit_BMI2
            ? CPU_FEATURE_X86_BMI2 : 0;


        if ((features & CPU_FEATURE_X86_AVX) &&
            (ebx & bit_AVX2)) {
            features |= CPU_FEATURE_X86_AVX2;
        }

        if ((ebx & bit_AVX512F) &&
            (xcr0 & 0xE6) == 0xE6) {

            features |= CPU_FEATURE_X86_AVX512F;

            features |= ebx & bit_AVX512BW
                ? CPU_FEATURE_X86_AVX512BW : 0;

            features |= ebx & bit_AVX512VL
                ? CPU_FEATURE_X86_AVX512VL : 0;

            features |= ebx & bit_AVX512DQ
                ? CPU_FEATURE_X86_AVX512DQ : 0;
        }
    }
}
#endif

    cpu_info->features = features;
}

cpu_info_t cpu_query_info(void)
{
    cpu_info_t cpu_info = {0};
    cpu__query_name(&cpu_info);
    cpu__get_cpu_name_cpuinfo(&cpu_info);
    cpu__query_cores(&cpu_info);
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

        [9]  = "sse2",
        [10] = "ssse3",
        [11] = "sse4.1",

        [12] = "avx",
        [13] = "avx2",
        [14] = "fma",

        [15] = "f16c",

        [16] = "bmi1",
        [17] = "bmi2",

        [18] = "popcnt",

        [19] = "avx512f",
        [20] = "avx512bw",
        [21] = "avx512vl",
        [22] = "avx512dq",
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

        [8] = "ARM Hardware atomics (LSE atomics)",

        [9]  = "x86 Streaming SIMD Extensions 2",
        [10] = "x86 Supplemental Streaming SIMD Extensions 3",
        [11] = "x86 Streaming SIMD Extensions 4.1",

        [12] = "x86 Advanced Vector Extensions",
        [13] = "x86 Advanced Vector Extensions 2",
        [14] = "x86 Fused Multiply-Add",

        [15] = "x86 half-precision conversion instructions",

        [16] = "x86 Bit Manipulation Instruction Set 1",
        [17] = "x86 Bit Manipulation Instruction Set 2",

        [18] = "x86 population count",

        [19] = "x86 AVX-512 Foundation",
        [20] = "x86 AVX-512 Byte and Word",
        [21] = "x86 AVX-512 Vector Length",
        [22] = "x86 AVX-512 Doubleword and Quadword",
    };
    
    if(bit_idx < 0 || bit_idx >= 32)
        return "unknown";
    return feature_descriptions[bit_idx] ? 
        feature_descriptions[bit_idx] : "unknown";
}

