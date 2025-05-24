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

#ifndef TD_BLACK_MAGIC_H
#define TD_BLACK_MAGIC_H

/**
 * @file td_black_magic.h
 * @brief Advanced macro utilities used internally by td_display to reduce boilerplate code.
 *
 * @warning This file contains macro-based metaprogramming. Proceed with caution.
 * @note This file is used for macro trickery such as variadic argument counting, automatic constructor generation, etc.
 */

// Don't do it XD

/**
 * @def __member_type(type, member)
 * @brief Resolves the type of a struct member.
 * @param type The type to inspect.
 * @param member The member to extract the type from.
 */
#define __member_type(type, member) __typeof__((type){0}.member)

/**
 * @def __cat(a, b)
 * @brief Concatenates two tokens.
 */
#define __cat(a, b) __cat_(a, b)
#define __cat_(a, b) a##b

/**
 * @def __count_args(...)
 * @brief Counts the number of arguments passed in.
 */
#define __count_args(...) __count_args_(__VA_ARGS__, 8,7,6,5,4,3,2,1)
/** @internal */
#define __count_args_(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N

/**
 * @def __expand(...)
 * @brief Forces macro expansion of arguments.
 */
#define __expand(...) __expand1(__expand1(__expand1(__VA_ARGS__)))
/** @internal */
#define __expand1(...) __VA_ARGS__

// Mapping macros over arguments (up to 8 arguments)
#define __map_1(m, a, x)               m(a, x)
#define __map_2(m, a, x, ...)          m(a, x), __map_1(m, a, __VA_ARGS__)
#define __map_3(m, a, x, ...)          m(a, x), __map_2(m, a, __VA_ARGS__)
#define __map_4(m, a, x, ...)          m(a, x), __map_3(m, a, __VA_ARGS__)
#define __map_5(m, a, x, ...)          m(a, x), __map_4(m, a, __VA_ARGS__)
#define __map_6(m, a, x, ...)          m(a, x), __map_5(m, a, __VA_ARGS__)
#define __map_7(m, a, x, ...)          m(a, x), __map_6(m, a, __VA_ARGS__)
#define __map_8(m, a, x, ...)          m(a, x), __map_7(m, a, __VA_ARGS__)

/**
 * @def __map(m, a, ...)
 * @brief Maps a macro `m` with fixed first argument `a` over variadic arguments.
 */
#define __map(m, a, ...) __cat(__map_, __count_args(__VA_ARGS__))(m, a, __VA_ARGS__)

/**
 * @def __arg(type, member)
 * @brief Expands to a typed member initializer parameter.
 */
#define __arg(type, member) __member_type(type, member) member

/**
 * @def __init(_, member)
 * @brief Expands to a named initializer using the field name.
 */
#define __init(_, member) .member = member

/**
 * @def __td_priv_create_constructor(name, type, ...)
 * @brief Generates a constructor function for a struct with field names.
 *
 * This creates a function of the form:
 * @code
 * td_rgba rgba_init(u8 r, u8 g, u8 b, u8 a) {
 *     return (td_rgba){ .r = r, .g = g, .b = b, .a = a };
 * }
 * @endcode
 *
 * @param name Function name
 * @param type Struct type
 * @param ... List of field names in the struct
 */
#define __td_priv_create_constructor(name, type, ...) \
    TD_INLINE type name(__expand(__map(__arg, type, __VA_ARGS__))) { \
        return (type){ __expand(__map(__init, _, __VA_ARGS__)) }; \
    }

#endif // TD_BLACK_MAGIC_H