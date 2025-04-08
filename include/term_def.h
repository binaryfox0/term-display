#ifndef TD_DEFINITION_H
#define TD_DEFINITION_H

#define EXPAND_RGBA(c) { (c).r, (c).g, (c).b, (c).a }
#define IN_RANGE(value, first, last) ((first) <= (value) && (value) <= (last))
#define OUT_RANGE(value, first, last) ((value) < (first) || (value) > (last))

#define __member_type(type, member) __typeof__((type){0}.member)

#define __cat(a, b) __cat_(a, b)
#define __cat_(a, b) a##b

#define __count_args(...) __count_args_(__VA_ARGS__, 8,7,6,5,4,3,2,1)
#define __count_args_(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N

#define __expand(...) __expand1(__expand1(__expand1(__VA_ARGS__)))
#define __expand1(...) __VA_ARGS__

#define __map_1(m, a, x)               m(a, x)
#define __map_2(m, a, x, ...)          m(a, x), __map_1(m, a, __VA_ARGS__)
#define __map_3(m, a, x, ...)          m(a, x), __map_2(m, a, __VA_ARGS__)
#define __map_4(m, a, x, ...)          m(a, x), __map_3(m, a, __VA_ARGS__)
#define __map_5(m, a, x, ...)          m(a, x), __map_4(m, a, __VA_ARGS__)
#define __map_6(m, a, x, ...)          m(a, x), __map_5(m, a, __VA_ARGS__)
#define __map_7(m, a, x, ...)          m(a, x), __map_6(m, a, __VA_ARGS__)
#define __map_8(m, a, x, ...)          m(a, x), __map_7(m, a, __VA_ARGS__)
#define __map(m, a, ...) __cat(__map_, __count_args(__VA_ARGS__))(m, a, __VA_ARGS__)

#define __arg(type, member) __member_type(type, member) member
#define __init(_, member) .member = member

#define __td_priv_create_constructor(name, type, ...) \
    TD_INLINE type name(__expand(__map(__arg, type, __VA_ARGS__))) { \
        return (type){ __expand(__map(__init, _, __VA_ARGS__)) }; \
    }

#if defined(_WIN64) || defined(_WIN32)
#   define TD_PLATFORM_WINDOWS
#endif

//https://stackoverflow.com/a/7063372/19703526
#if defined(unix) || defined(__unix__) || defined(__unix)
#   define TD_PLATFORM_UNIX
#endif

#if defined(_MSC_VER)
#   define TD_INLINE __forceinline
#else
#   define TD_INLINE static inline __attribute((always_inline))
#endif

typedef char term_i8;
typedef unsigned char term_u8;
typedef unsigned short term_u16;
typedef int term_i32;
typedef unsigned int term_u32;
typedef unsigned long long term_u64;
typedef float term_f32;
typedef enum { term_false = 0, term_true } term_bool;

#include "term_math.h"

typedef struct {
    term_u8 r, g, b;
} term_rgb;
typedef struct {
    term_u8 r, g, b, a;
} term_rgba;

/* Structure initializer begin */
__td_priv_create_constructor(rgba_init, term_rgba, r, g, b, a)
__td_priv_create_constructor(rgb_init, term_rgb, r, g, b)

TD_INLINE term_rgba to_rgba(const term_rgb c)
{
    return (term_rgba) {
    .r = c.r,.g = c.g,.b = c.b,.a = 255};
}

TD_INLINE term_rgb to_rgb(const term_rgba c)
{
    return (term_rgb) {
    .r = c.r,.g = c.g,.b = c.b};
}

/* Structure initializer end */

#endif
