/*
MIT License

Copyright (c) 2026 binaryfox0 (Duy Pham Duc)

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

#ifndef TD_MISC_H
#define TD_MISC_H

#include <td_def.h>
#include <td_error.h>

#if defined(__clang__) || defined(__GNUC__)
#   define __TD_PRINTF_LIKE__(fmt_idx, first_arg_idx) \
    __attribute__((format(printf, fmt_idx, first_arg_idx)))
#else
#   define __TD_PRINTF_LIKE__(fmt_idx, first_arg_idx)
#endif

td_error_t td_debug_set(const char *name);
__TD_PRINTF_LIKE__(1, 2) td_error_t td_debug_log(const char *fmt, ...);

#endif
