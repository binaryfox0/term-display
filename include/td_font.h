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
 * @brief Text rendering utilities for term-display.
 *
 * This module provides APIs to convert characters and strings into
 * texture objects that can be rendered with term-display.
 */

#ifndef TD_FONT_H
#define TD_FONT_H

#include <td_def.h>
#include <td_error.h>
#include <td_texture.h>

/**
 * @brief Opaque font object.
 *
 * Holds glyph data and rendering configuration.
 */
typedef struct td_font td_font;

/**
 * @brief Create a new empty font object.
 *
 * The returned font contains no glyphs until populated.
 *
 * @return Pointer to a newly allocated font, or NULL on failure.
 */
td_font* td_create_font(void);

/**
 * @brief Create a default font with predefined colors.
 *
 * @param foreground Default text color.
 * @param background Default background color.
 * @return Pointer to initialized font instance.
 */
td_font* td_default_font(const td_rgba foreground, const td_rgba background);

/**
 * @brief Destroy and free a font object.
 *
 * @param font Font instance to destroy.
 */
void td_destroy_font(td_font* font);

/**
 * @brief Insert a glyph texture into the font atlas.
 *
 * Associates a Unicode codepoint with a pre-rendered texture.
 *
 * @param font Font container.
 * @param codepoint Unicode codepoint of the character.
 * @param tex Texture representing the glyph.
 * @return Error code indicating success or failure.
 */
td_error_t td_font_insert_char(
        td_font *font,
        const td_i32 codepoint,
        const td_texture_t *tex
);

/**
 * @brief Calculate pixel size of a text string.
 *
 * Computes width and height required to render the given string
 * using the specified font.
 *
 * @param font Font used for measurement.
 * @param str Input string.
 * @param str_len Length of the string.
 * @return 2D vector containing width (x) and height (y).
 */
td_ivec2 td_calc_text_size(const td_font *font, const char *str, const td_u64 str_len);

/**
 * @brief Render a single character into a texture.
 *
 * @param font Font used for rendering.
 * @param ch Unicode codepoint of the character.
 * @return Newly allocated texture containing rendered glyph.
 */
td_texture_t *td_render_char(const td_font* font, const td_i32 ch);

/**
 * @brief Render a string into an existing texture buffer.
 *
 * Writes rendered text into `tex_out` starting at the given position.
 *
 * @param font Font used for rendering.
 * @param pos Top-left position in destination texture.
 * @param str Input string.
 * @param str_len Length of string.
 * @param tex_out Output texture buffer.
 * @return Error code indicating success or failure.
 */
td_error_t td_render_string_into(
        const td_font *font,
        const td_ivec2 pos,
        const char *str,
        const td_u64 str_len,
        td_texture_t *tex_out
);

/**
 * @brief Render a string into a new texture.
 *
 * Creates a texture containing the full rendered string.
 *
 * @param font Font used for rendering.
 * @param str Null-terminated input string.
 * @param len Length of the string.
 * @return Newly allocated texture containing rendered text.
 */
td_texture_t *td_render_string(
        const td_font* font,
        const char *str,
        const td_u64 len
);

#endif // TD_FONT_H
