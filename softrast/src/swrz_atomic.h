#ifndef SWRZ_ATOMIC_H
#define SWRZ_ATOMIC_H

#include <stdint.h>
#include <stdbool.h>

typedef int32_t swrz__atomic_int_t;
#if defined(__clang__) || defined(__GNUC__)
#   define swrz__atomic_init(value) \
        ((swrz__atomic_int_t)(value))

#   define swrz__atomic_load(ptr) \
        __atomic_load_n((ptr), __ATOMIC_ACQUIRE)

#   define swrz__atomic_store(ptr, value) \
        __atomic_store_n((ptr), (value), __ATOMIC_RELEASE)

#elif defined(_MSC_VER)
#   include <intrin.h>

#   define swrz__atomic_init(value) \
        ((swrz__atomic_int_t)(value))

#   define swrz__atomic_load(ptr) \
        InterlockedCompareExchange( \
                (volatile LONG *)(ptr), 0, 0)

#   define swrz__atomic_store(ptr, value) \
        InterlockedExchange( \
                (volatile LONG *)(ptr), (LONG)(value))
#else
#   error "softrast doesn't support this compiler, please file an issue on Github"
#endif

#endif
