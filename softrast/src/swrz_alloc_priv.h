#ifndef SWRZ_ALLOC_PRIV_H
#define SWRZ_ALLOC_PRIV_H

#include <stddef.h>

void *swrz__malloc(
        size_t size);

void *swrz__realloc(
        void *ptr, 
        size_t size);

void swrz__free(
        void *ptr);

#endif
