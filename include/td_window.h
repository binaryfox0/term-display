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
#ifndef TD_WINDOW_H
#define TD_WINDOW_H

#include <td_error.h>
#include <td_def.h>

typedef struct td_window td_window_t;

typedef enum td_window_flags
{
    TD_WINDOW_FULLSCREEN = (1 << 0),
    TD_WINDOW_RESIZABLE = (1 << 1)
} td_window_flags_t;

/*
 * @brief Terminal color rendering modes.
 *
 * Defines the available output color capabilities used by the renderer.
 * These modes control how colors are quantized and emitted to the target
 * terminal or display backend.
 */
typedef enum td_color_mode
{
    TD_COLOR_GRAYSCALE_24,   /**< 24-level grayscale output. */
    TD_COLOR_GRAYSCALE_256,  /**< 256-level grayscale output. */
    TD_COLOR_ANSI_216,       /**< 216-color ANSI palette output. */
    TD_COLOR_TRUECOLOR,      /**< Full 24-bit RGB truecolor output. */
    __TD_COLOR_MAX__
} td_color_mode_t;

td_window_t *td_window_create(
        const td_i32 x,
        const td_i32 y,
        const td_i32 w,
        const td_i32 h,
        const td_i32 rotation,
        const td_color_mode_t color_mode,
        const td_window_flags_t flags
);

td_error_t td_window_resize(
        td_window_t *window,
        const td_ivec2 logical_size);

td_error_t td_window_set_position(
        td_window_t *window,
        const td_i32 x,
        const td_i32 y);

td_error_t td_window_set_size(
        td_window_t *window,
        const td_i32 width,
        const td_i32 height);

td_error_t td_window_set_orientation(
        td_window_t *window,
        const td_i32 orientation);

td_error_t td_window_set_resizable(
        td_window_t *window,
        const td_bool resizable);

td_error_t td_window_set_should_close(
        td_window_t *window,
        const td_bool should_close);

td_window_flags_t td_window_get_flags(
        td_window_t *window);

td_error_t td_window_get_position(
        td_window_t *window,
        td_i32 *x,
        td_i32 *y);

td_error_t td_window_get_size(
        td_window_t *window,
        td_i32 *width,
        td_i32 *height);

td_i32 td_window_get_orientation(
        td_window_t *window);

td_bool td_window_should_close(
        td_window_t *window);

td_error_t td_window_present(
        td_window_t *window);

void td_window_destroy(td_window_t *window);

#endif
