#ifndef SWRZ_UTILS_H
#define SWRZ_UTILS_H

#define SWRZ__ENUM_IN_RANGE(prefix, value) \
    ((value) >= 0 && (value) < SWRZ__##prefix##_COUNT)
#define SWRZ__CHECK(x, err, label) \
    if(((err) = x) != SWRZ_ERR_OK) \
        goto label
#define SWRZ__CLAMP(x, min, max) \
    ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#endif
