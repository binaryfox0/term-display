#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define MAX_CPU_NAME_LENGTH 128
#define MAX_CPU_VENDOR_LENGTH 32
#define MAX_CLUSTER_COUNT 8
#define MAX_CPU_PER_CLUSTER_COUNT 32

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

cpu_info_t cpu_query_info(void);
const char *cpu_get_feature_name(
        const int bit_idx);
const char *cpu_get_feature_description(
        const int bit_idx);

#endif
