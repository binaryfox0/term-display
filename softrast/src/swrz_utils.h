#ifndef SWRZ_UTILS_H
#define SWRZ_UTILS_H

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

#endif
