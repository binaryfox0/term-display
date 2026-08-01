#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>

#include <stdlib.h>
#include <sys/system_properties.h>

#include <aparse.h>

#define info aparse_prog_info
#define warn aparse_prog_warn
#define error aparse_prog_error

#define CPU_NAME_BUFSIZ 128
#define CPU_VENDOR_BUFSIZ 32

typedef struct
{
    char name[CPU_NAME_BUFSIZ];
    char vendor[CPU_VENDOR_BUFSIZ];
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
        return true;
    if(__system_property_get(
                "ro.product.product.manufacturer", prop_value) > 0)
        return true;

    if(__system_property_get("ro.product.vendor.manufacturer", 
                prop_value) > 0)
        return true;

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
            str_copy(cpu_info->name, sizeof(cpu_info->name), value);
        
    }

    fclose(file);
}

int main(int argc, char **argv)
{
    cpu_info_t cpu_info = {0};
    // struct utsname ustbuf = {0};
    // uname(&ustbuf);

    aparse_parse(argc, argv, NULL, NULL, NULL);
    
    get_android_cpu(&cpu_info);
    parse_cpuinfo(&cpu_info);

    info("vendor: %s", cpu_info.vendor);
    info("name: %s", cpu_info.name);
    
    return 0;
}
