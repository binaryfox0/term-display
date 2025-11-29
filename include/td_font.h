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
 * @file td_font.h
 * @brief Font rendering utilities for term-display.
 *
 * Provides basic functionality for converting characters and strings
 * into textured representations.
 */

/**
 * @defgroup td_font Font Utilities
 * @brief Character and string to texture conversion.
 *
 * These functions allow rendering individual characters and strings
 * as `td_texture` objects using specified foreground/background colors.
 *
 * @{
 */

#ifndef TD_FONT_H
#define TD_FONT_H

#include "td_def.h"
#include "td_texture.h"

typedef struct td_font td_font;

td_font* td_create_font(void);
td_font* td_default_font(const td_rgba foreground, const td_rgba background);
void td_destroy_font(td_font* font);
td_bool td_add_character(td_font* font, const td_ivec2 range, const td_texture* tex_atlas);

/**
 * @brief Generate a texture from a single character.
 *
 * @param font A font was created with \ref td_font_create
 * @param ch The character to render.
 * @param fg Foreground (text) color.
 * @param bg Background color.
 * @return A pointer to a new `td_texture` representing the character.
 */
td_texture *td_render_char(const td_font* font, const td_i32 ch);

/**
 * @brief Generate a texture from a string.
 *
 * @param str Null-terminated string to render.
 * @param len Length of the string.
 * @param size Optional output for texture size (can be NULL).
 * @param fg Foreground (aka text) color.
 * @param bg Background color.
 * @return A pointer to a new `td_texture` containing the rendered string.
 */
td_texture *td_render_string(const td_font* font,
                              const char *str, const td_u32 len,
                              td_ivec2 *size);

/** @} */ // end of td_font

#endif // TD_FONT_H
