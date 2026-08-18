#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define MAX_CPU_NAME_LENGTH 128
#define MAX_CPU_ARCH_LENGTH 16
#define MAX_CLUSTER_COUNT 16
#define MAX_CPU_PER_CLUSTER_COUNT 32

enum
{
    CPU_FEATURE_ARM_FPHP       = 1u << 0,
    CPU_FEATURE_ARM_FP         = 1u << 1,

    CPU_FEATURE_ARM_ASIMDHP    = 1u << 2,
    CPU_FEATURE_ARM_ASIMDFHM   = 1u << 3,
    CPU_FEATURE_ARM_ASIMD      = 1u << 4,
    CPU_FEATURE_ARM_ASIMDDP    = 1u << 5,

    CPU_FEATURE_ARM_SVE        = 1u << 6,
    CPU_FEATURE_ARM_SVE2       = 1u << 7,

    CPU_FEATURE_ARM_ATOMICS    = 1u << 8,

    CPU_FEATURE_X86_SSE2       = 1u << 9,
    CPU_FEATURE_X86_SSSE3      = 1u << 10,
    CPU_FEATURE_X86_SSE41      = 1u << 11,
    CPU_FEATURE_X86_SSE42      = 1u << 12,

    CPU_FEATURE_X86_AVX        = 1u << 13,
    CPU_FEATURE_X86_AVX2       = 1u << 14,
    CPU_FEATURE_X86_FMA        = 1u << 15,

    CPU_FEATURE_X86_F16C       = 1u << 16,

    CPU_FEATURE_X86_BMI1       = 1u << 17,
    CPU_FEATURE_X86_BMI2       = 1u << 18,

    CPU_FEATURE_X86_POPCNT     = 1u << 19,

    CPU_FEATURE_X86_AVX512F    = 1u << 20,
    CPU_FEATURE_X86_AVX512BW   = 1u << 21,
    CPU_FEATURE_X86_AVX512VL   = 1u << 22,
    CPU_FEATURE_X86_AVX512DQ   = 1u << 23,
};

typedef struct
{
    char name[MAX_CPU_NAME_LENGTH];
    char arch[MAX_CPU_ARCH_LENGTH];

    uint16_t core_logical;
    uint16_t core_online;
    uint32_t l1_size;
    uint32_t l2_size;
    uint32_t l3_size;
    uint32_t features;

    struct
    {
        uint16_t max_mhz;
        uint16_t cpus[MAX_CPU_PER_CLUSTER_COUNT];
        int cpu_count;
    } cluster[MAX_CLUSTER_COUNT];
    int cluster_count;
    
} cpu_info_t;

cpu_info_t cpu_query_info(void);
const char *cpu_get_feature_name(
        const int bit_idx);
const char *cpu_get_feature_description(
        const int bit_idx);

#endif
