#ifndef SWRZ_UTILS_H
#define SWRZ_UTILS_H

#include <stdint.h>
#if defined(_MSC_VER)
#   include <intrin.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#   define SWRZ__INLINE static inline __attribute__((always_inline))
#   define SWRZ__ALIGN(n) __attribute__((align(n)))
#elif defined(_MSC_VER)
#   define SWRZ__INLINE __forceinline
#   define SWRZ__ALIGN(n) __declspec(align(n))
#else
#   define SWRZ__INLINE
#   define SWRZ__ALIGN(n)
#endif

#define SWRZ__IN_RANGE(value, begin, end) \
    ((value) >= (begin) && (value) <= (end))

#define SWRZ__ENUM_IN_RANGE(prefix, value) \
    ((value) >= 0 && (value) < SWRZ__##prefix##_COUNT)

#define SWRZ__CHECK(x, err, label) \
    if(((err) = x) != SWRZ_ERR_OK) \
        goto label

#define SWRZ__CLAMP(x, min, max) \
    ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define SWRZ__MIN(a, b) ((a) < (b) ? (a) : (b))
#define SWRZ__MAX(a, b) ((a) > (b) ? (a) : (b))
#define SWRZ__MIN3(a, b, c) SWRZ__MIN(SWRZ__MIN((a), (b)), (c))
#define SWRZ__MAX3(a, b, c) SWRZ__MAX(SWRZ__MAX((a), (b)), (c))

#define SWRZ__CEILDIV(a, b) (((a) + (b) - 1) / (b))

SWRZ__INLINE int swrz__ffs(
        uint32_t x)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctz(x);

#elif defined(_MSC_VER)
    unsigned long i = 0;
    _BitScanForward(&i, x);
    return (int)i;

#else
    int n = 0;
    if ((x & 0x0000FFFFu) == 0)
        x >>= 16, n += 16
    if ((x & 0x000000FFu) == 0)
        x >>= 8, n += 8;
    if ((x & 0x0000000Fu) == 0)
        x >>= 4, n += 4;
    if ((x & 0x00000003u) == 0)
        x >>= 2, n += 2;
    if ((x & 0x00000001u) == 0)
        n += 1;

    return n;
#endif
}

#endif
