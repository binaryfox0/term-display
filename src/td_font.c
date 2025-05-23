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

#include "td_font.h"

#include <stdlib.h>
#include <string.h>

// Macro (cuz c don't allow using const in size)
#define ATLAS_SIZE 69
#define LOWER_LIMIT ' '
#define UPPER_LIMIT '~'
#define CHAR_WIDTH 3
#define CHAR_HEIGHT 5
#define CHAR_PIXEL CHAR_WIDTH * CHAR_HEIGHT

// Atlas of font character pattern, will be populated later
static const td_u8 texture_atlas[ATLAS_SIZE][CHAR_PIXEL] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // Space
    { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0 },    // Exclamation mark
    { 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // Quotation mark
    { 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1 },    // Number sign
    { 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0 },    // Dollar sign
    { 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1 },    // Percent sign
    { 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1 },    // Ampersand
    { 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // Astrophobe
    { 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0 },    // Left parenthesis
    { 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0 },    // Right parenthesis
    { 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1 },    // Asterisk
    { 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0 },    // Plus sign
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0 },    // Comma
    { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },    // Hyphen
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },    // Period
    { 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0 },    // Slash
    { 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1 },    // Digit 0
    { 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 },    // Digit 1
    { 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1 },    // Digit 2
    { 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0 },    // Digit 3
    { 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1 },    // Digit 4
    { 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0 },    // Digit 5
    { 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0 },    // Digit 6
    { 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 },    // Digit 7
    { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 },    // Digit 8
    { 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0 },    // Digit 9
    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },    // Colon
    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0 },    // Semicolon
    { 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },    // Less-than
    { 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0 },    // Equals-to
    { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0 },    // Greater-than
    { 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0 },    // Question mark
    { 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1 },    // At sign
    { 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1 },    // Letter A
    { 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0 },    // Letter B
    { 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0 },    // Letter C
    { 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0 },    // Letter D
    { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1 },    // Letter E
    { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0 },    // Letter F
    { 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1 },    // Letter G
    { 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1 },    // Letter H
    { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 },    // Letter I
    { 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0 },    // Letter J
    { 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1 },    // Letter K
    { 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1 },    // Letter L
    { 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1 },    // Letter M
    { 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1 },    // Letter N
    { 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0 },    // Letter O
    { 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0 },    // Letter P
    { 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1 },    // Letter Q
    { 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1 },    // Letter R
    { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 },    // Letter S
    { 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 },    // Letter T
    { 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1 },    // Letter U
    { 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0 },    // Letter V
    { 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1 },    // Letter W
    { 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1 },    // Letter X
    { 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 },    // Letter Y
    { 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1 },    // Letter Z
    { 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0 },    // Left square bracket
    { 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1 },    // Backslash
    { 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1 },    // Right square bracket
    { 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // Caret
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },    // Underscore 
    { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    // Grave accent
    { 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1 },    // Left curly brace
    { 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0 },    // Vertical bar
    { 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0 },    // Right curly brace
    { 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }     // Tilde 
};

TD_INLINE td_u32 calculate_pad(td_u32 ch)
{
    return (ch ? ch - 1 : 0);
}

td_u8 is_newline(const td_i8 *str, td_u32 *current, td_u32 max)
{
    td_u8 cr = 0;                  // CR LF detection
    if (str[*current] == '\n' || (cr = (str[*current] == '\r'))) {
        if (cr && *current + 1 < max && str[*current + 1] == '\n')
            (*current)++;
        return 1;
    }
    return 0;
}

td_u8 query_newline(const td_i8 *str, td_u32 len, td_u32 **lines_length,    // Lines length info
                 td_u32 *line_count, td_u32 *longest_line)
{
    static const size_t min_realloc = 10;

    td_u32 line_len = 0, current_size = 0;
    *line_count = 0;
    *longest_line = 0;

    for (td_u32 i = 0; i < len && str[i]; i++) {
        if (is_newline(str, &i, len)) {
            if (*line_count >= current_size) {
                size_t new_size = (size_t)current_size + min_realloc;
                td_u32 *temp =
                    (td_u32 *) realloc(*lines_length, new_size * sizeof(td_i32));
                if (!temp)
                    return 1;   // Allocation failed
                *lines_length = temp;
                current_size = (td_u32)new_size;
            }

            (*lines_length)[*line_count] = line_len;
            if (line_len > *longest_line)
                *longest_line = line_len;
            (*line_count)++;
            line_len = 0;
            continue;
        }
        line_len++;
    }

    // Handle last line if it exists
    if (line_len > 0) {
        if (*line_count >= current_size) {
            size_t new_size = (size_t)(current_size + 1);    // Allocate just for the last line
            td_u32 *temp =
                (td_u32 *) realloc(*lines_length, new_size * sizeof(td_i32));
            if (!temp)
                return 1;
            *lines_length = temp;
        }
        (*lines_length)[*line_count] = line_len;
        if (line_len > *longest_line)
            *longest_line = line_len;
        (*line_count)++;
    }

    return 0;
}


TD_INLINE td_i8 mapped_ch(td_i8 ch)
{
    if (ch >= 'a')
        return ch - 'a' + 'A';  // To uppercase (how this font mapped)
    return ch;
}

td_texture *tdf_char_texture(td_i8 ch, td_rgba color, td_rgba fg)
{
    ch = mapped_ch(ch);
    if (!IN_RANGE(ch, LOWER_LIMIT, UPPER_LIMIT))
        ch = LOWER_LIMIT;       // Space (nothing)
    const td_u8 *ch_template = texture_atlas[ch - LOWER_LIMIT];
    td_texture *out =
        tdt_create(0, 4, td_ivec2_init(CHAR_WIDTH, CHAR_HEIGHT), 0, 0);
    td_u8 *raw = tdt_get_location(td_ivec2_init(0, 0), out);
    td_u8 a[4] = EXPAND_RGBA(color), b[4] = EXPAND_RGBA(fg);
    for (td_u8 i = 0; i < CHAR_PIXEL; i++)
        for (td_u8 j = 0; j < 4; j++, raw++)
            raw[0] = ch_template[i] ? a[j] : b[j];

    return out;
}

td_texture *tdf_string_texture(const td_i8 *str, td_u32 len,
                                     td_ivec2 *s,
                                     td_rgba color, td_rgba fg)
{
    if (!str || !len || !s)
        return 0;
    td_u32 plen = (td_u32)len;
    td_u32 lines_count = 0, longest_line = 0;
    td_u32 *lines_length = 0;      // Filling gaps
    if (query_newline
        (str, plen, &lines_length, &lines_count, &longest_line)) {
        return 0;
    }
    // s->x is maximum character in one line, s->y is the line in the input text
    s->x = (int)(longest_line * CHAR_WIDTH + calculate_pad(longest_line));
    s->y = (int)(lines_count * CHAR_HEIGHT + calculate_pad(lines_count));
    td_texture *out = tdt_create(0, 4, *s, 0, 0);
    if (!out) {
        free(lines_length);
        return 0;
    }
    tdt_fill(out, fg);

    // Placing character into place
    td_u32 current_index = 0;
    for (td_u32 row = 0; row < lines_count; row++) {
        td_u32 row_l = lines_length[row], start_y = row * (CHAR_HEIGHT + 1);       // 1 is pad
        for (td_u32 col = 0; col < row_l; col++) {
            td_texture *ch_texture =
                tdf_char_texture(str[current_index + col], color, fg);
            td_u32 start_x = col * (CHAR_WIDTH + 1);
            tdt_merge(out, ch_texture, td_ivec2_init((int)start_x, (int)start_y),
                          TEXTURE_MERGE_CROP, 1);
            tdt_free(ch_texture);
        }
        current_index += row_l; // Now at newline characater
        if (is_newline(str, &current_index, plen))
            current_index++;    // Skip newline now
    }
    free(lines_length);
    return out;
}
