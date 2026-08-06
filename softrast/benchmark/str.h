#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <stdbool.h>

void str_copy(
        char *dst, 
        const size_t dst_size, 
        const char *src);

bool str_starts_with(
        const char *str, 
        const char *prefix);

bool str_empty(
        const char *str);

bool str_compare(
        const char *a, 
        const char *b);

bool str_compare_ignore(
        const char *a, 
        const char *b);

#endif
