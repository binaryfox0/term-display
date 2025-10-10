/*
MIT License

Copyright (c) 2025 binaryfox0 (Duy Pham Duc)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef TD_DEFINITION_H
#define TD_DEFINITION_H

/**
 * @file td_def.h
 * @brief Core type definitions, platform macros, and color utilities for the td_display system.
 */

#include "td_black_magic.h"

/**
 * @def TD_EXPAND_RGBA(c)
 * @brief Expands a `td_rgba` struct into its four channel values.
 * @param c The `td_rgba` struct to expand.
 */
#define TD_EXPAND_RGBA(c) { (c).r, (c).g, (c).b, (c).a }

/**
 * @def IN_RANGE(value, first, last)
 * @brief Checks if a value lies within an inclusive range.
 */
#define IN_RANGE(value, first, last) ((first) <= (value) && (value) <= (last))

/**
 * @def OUT_RANGE(value, first, last)
 * @brief Checks if a value lies outside an inclusive range.
 */
#define OUT_RANGE(value, first, last) ((value) < (first) || (value) > (last))

// Platform detection
#if defined(_WIN64) || defined(_WIN32)
    /**
     * @def TD_PLATFORM_WINDOWS
     * @brief Defined if compiling on a Windows platform.
     */
#   define TD_PLATFORM_WINDOWS
#endif

#if defined(unix) || defined(__unix__) || defined(__unix)
    /**
     * @def TD_PLATFORM_UNIX
     * @brief Defined if compiling on a Unix-like platform.
     */
#   define TD_PLATFORM_UNIX
#endif

#if defined(_MSC_VER)
    /**
     * @def TD_INLINE
     * @brief Platform-specific inline macro for MSVC.
     */
#   define TD_INLINE __forceinline
#else
    /**
     * @def TD_INLINE
     * @brief Platform-specific inline macro for GCC/Clang.
     */
#   define TD_INLINE static inline __attribute((always_inline))
#endif

/** @typedef td_i8
 * @brief 8-bit signed integer.
 */
typedef char td_i8;

/** @typedef td_u8
 * @brief 8-bit unsigned integer.
 */
typedef unsigned char td_u8;

/** @typedef td_u16
 * @brief 16-bit unsigned integer.
 */
typedef unsigned short td_u16;

/** @typedef td_i32
 * @brief 32-bit signed integer.
 */
typedef int td_i32;

/** @typedef td_u32
 * @brief 32-bit unsigned integer.
 */
typedef unsigned int td_u32;

/** @typedef td_u64
 * @brief 64-bit unsigned integer.
 */
typedef unsigned long long td_u64;

/** @typedef td_f32
 * @brief 32-bit floating point number.
 */
typedef float td_f32;

/**
 * @enum td_bool
 * @brief Boolean type for true/false values.
 */
typedef enum {
    td_false = 0, ///< Boolean false
    td_true       ///< Boolean true
} td_bool;

/**
 * @struct td_rgb
 * @brief RGB color type with three 8-bit components.
 */
typedef struct {
    struct {
        td_u8 r; ///< Red component
        td_u8 g; ///< Green component
        td_u8 b; ///< Blue component
    };
    td_u8 raw[3]; ///< Raw access to RGB bytes
} td_rgb;

/**
 * @union td_rgba
 * @brief RGBA color type with four 8-bit components.
 */
typedef union {
    struct {
        td_u8 r; ///< Red component
        td_u8 g; ///< Green component
        td_u8 b; ///< Blue component
        td_u8 a; ///< Alpha (transparency) component
    };
    td_u8 raw[4]; ///< Raw access to RGBA bytes
} td_rgba;

/**
 * @brief Creates a td_rgba struct from individual values.
 */
__td_priv_create_constructor(td_rgba_init, td_rgba, r, g, b, a)

/**
 * @brief Creates a td_rgb struct from individual values.
 */
__td_priv_create_constructor(td_rgb_init, td_rgb, r, g, b)

/**
 * @brief Converts a td_rgb color to td_rgba with alpha set to 255.
 * @param c The td_rgb color to convert.
 * @return td_rgba equivalent.
 */
TD_INLINE td_rgba to_rgba(const td_rgb c)
{
    return (td_rgba){ .r = c.r, .g = c.g, .b = c.b, .a = 255 };
}

/**
 * @brief Converts a td_rgba color to td_rgb, dropping the alpha.
 * @param c The td_rgba color to convert.
 * @return td_rgb equivalent.
 */
TD_INLINE td_rgb to_rgb(const td_rgba c)
{
    return (td_rgb){ .r = c.r, .g = c.g, .b = c.b };
}

typedef struct {
    int x, y;
} td_ivec2;
typedef union {
    struct {
        float x, y;
    };
    float raw[2];
} td_vec2;
typedef union {
    struct {
        float x, y, z;
    };
    float raw[3];
} td_vec3;
typedef struct {
    td_u32 x, y, z;
} td_ivec3;

typedef struct {
    td_u32 left, top, right, bottom;
} td_rect;

__td_priv_create_constructor(td_ivec2_init, td_ivec2, x, y)
__td_priv_create_constructor(td_vec2_init, td_vec2, x, y)

#define td_ivec2_expand(v) (v).x, (v).y

TD_INLINE td_bool td_ivec2_is_zero(const td_ivec2 v) {
    return v.x == 0 || v.y == 0;
}

TD_INLINE td_ivec2 td_ivec2_subtract(const td_ivec2 a, const td_ivec2 b) {
    return td_ivec2_init(a.x - b.x, a.y - b.y);
}

TD_INLINE td_u8 ivec2_equal(const td_ivec2 a, const td_ivec2 b)
{
    return a.x == b.x && a.y == b.y;
}

TD_INLINE td_ivec2 ndc_to_pos(td_vec2 pos, td_ivec2 size)
{
    return td_ivec2_init((td_i32) ((pos.x + 1.0) * 0.5f * size.x),
                      (td_i32) ((1.0 - pos.y) * 0.5f * size.y)
        );
}

TD_INLINE td_vec2 pos_to_ndc(td_ivec2 pos, td_ivec2 size)
{
    td_vec2 ndc;
    ndc.x = (2.0f * (float)pos.x) / (float)(size.x - 1) - 1.0f;
    ndc.y = 1.0f - (2.0f * (float)pos.y) / (float)(size.y - 1);
    return ndc;
}

#endif // TD_DEFINITION_H
