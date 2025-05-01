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


#endif