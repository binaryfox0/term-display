#include "str.h"

#include <string.h>
#include <ctype.h>

void str_copy(
        char *dst, 
        const size_t dst_size, 
        const char *src)
{
    size_t i = 0;
    if (dst == NULL || src == NULL || dst_size == 0)
        return;

    while ((i < (dst_size - 1)) && (src[i] != '\0')) {
        dst[i] = src[i];
        ++i;
    }

    dst[i] = '\0';
}

bool str_starts_with(
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

bool str_empty(
        const char *str) {
    return str[0] == '\0';
}

bool str_compare(
        const char *a, 
        const char *b) {
    return strcmp((a), (b)) == 0;
}

bool str_compare_ignore(
        const char *a, 
        const char *b)
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
