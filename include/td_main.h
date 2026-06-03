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

/**
 * @file td_main.h
 * @brief Core declarations for the term-display library.
 *
 * This header provides the primary initialization and shutdown functions for
 * the term-display library. It also includes access to the library's core
 * modules, including events, rendering, textures, fonts, and common type
 * definitions.
 */

#ifndef TD_MAIN_H
#define TD_MAIN_H

#include <td_def.h>
#include <td_window.h>
#include <td_event.h>
#include <td_renderer.h>
/*
#include <td_event.h>
#include <td_texture.h>
#include <td_font.h>
*/
/**
 * @brief Returns the term-display copyright notice.
 *
 * The returned string is statically allocated and must not be modified
 * or freed by the caller.
 *
 * @return Pointer to the copyright notice string.
 */
const char *td_copyright_notice(void);

/**
 * @brief Initializes the term-display library.
 *
 * Initializes all required subsystems and prepares the library for use.
 * This function must be called before using any other term-display APIs.
 *
 * @return Operation result.
 */
td_error_t td_init(void);

/**
 * @brief Shuts down the term-display library.
 *
 * Releases resources allocated by the library and shuts down all initialized
 * subsystems. After calling this function, no other term-display APIs should
 * be used unless the library is initialized again with td_init().
 */
void td_quit(void);

#endif /* TD_MAIN_H */
