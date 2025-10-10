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

#define TDF_COMPRESS(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
    { \
        (a) | (b << 1) | (c << 2) | (d << 3) | (e << 4) | (f << 5) | (g << 6) | (h << 7), \
        (i) | (j << 1) | (k << 2) | (l << 3) | (m << 4) | (n << 5) | (o << 6) \
    }

// Atlas of font character pattern, will be populated later
static const td_u8 texture_atlas[ATLAS_SIZE][2] = {
    TDF_COMPRESS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),    // Space
    TDF_COMPRESS(0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0),    // Exclamation mark
    TDF_COMPRESS(1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),    // Quotation mark
    TDF_COMPRESS(1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1),    // Number sign
    TDF_COMPRESS(0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0),    // Dollar sign
    TDF_COMPRESS(1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1),    // Percent sign
    TDF_COMPRESS(0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1),    // Ampersand
    TDF_COMPRESS(0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),    // Astrophobe
    TDF_COMPRESS(0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0),    // Left parenthesis
    TDF_COMPRESS(0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0),    // Right parenthesis
    TDF_COMPRESS(1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1),    // Asterisk
    TDF_COMPRESS(0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0),    // Plus sign
    TDF_COMPRESS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0),    // Comma
    TDF_COMPRESS(0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0),    // Hyphen
    TDF_COMPRESS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0),    // Period
    TDF_COMPRESS(0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0),    // Slash
    TDF_COMPRESS(1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1),    // Digit 0
    TDF_COMPRESS(0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0),    // Digit 1
    TDF_COMPRESS(1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1),    // Digit 2
    TDF_COMPRESS(1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0),    // Digit 3
    TDF_COMPRESS(1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1),    // Digit 4
    TDF_COMPRESS(1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0),    // Digit 5
    TDF_COMPRESS(0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0),    // Digit 6
    TDF_COMPRESS(1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0),    // Digit 7
    TDF_COMPRESS(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0),    // Digit 8
    TDF_COMPRESS(0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0),    // Digit 9
    TDF_COMPRESS(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0),    // Colon
    TDF_COMPRESS(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0),    // Semicolon
    TDF_COMPRESS(0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1),    // Less-than
    TDF_COMPRESS(0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0),    // Equals-to
    TDF_COMPRESS(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0),    // Greater-than
    TDF_COMPRESS(1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0),    // Question mark
    TDF_COMPRESS(0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1),    // At sign
    TDF_COMPRESS(0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1),    // Letter A
    TDF_COMPRESS(1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0),    // Letter B
    TDF_COMPRESS(0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0),    // Letter C
    TDF_COMPRESS(1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0),    // Letter D
    TDF_COMPRESS(1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1),    // Letter E
    TDF_COMPRESS(1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0),    // Letter F
    TDF_COMPRESS(0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1),    // Letter G
    TDF_COMPRESS(1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1),    // Letter H
    TDF_COMPRESS(0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0),    // Letter I
    TDF_COMPRESS(0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0),    // Letter J
    TDF_COMPRESS(1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1),    // Letter K
    TDF_COMPRESS(1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1),    // Letter L
    TDF_COMPRESS(1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1),    // Letter M
    TDF_COMPRESS(1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1),    // Letter N
    TDF_COMPRESS(0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0),    // Letter O
    TDF_COMPRESS(1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0),    // Letter P
    TDF_COMPRESS(0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1),    // Letter Q
    TDF_COMPRESS(1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1),    // Letter R
    TDF_COMPRESS(0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0),    // Letter S
    TDF_COMPRESS(1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0),    // Letter T
    TDF_COMPRESS(1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1),    // Letter U
    TDF_COMPRESS(1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0),    // Letter V
    TDF_COMPRESS(1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1),    // Letter W
    TDF_COMPRESS(1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1),    // Letter X
    TDF_COMPRESS(1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0),    // Letter Y
    TDF_COMPRESS(1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1),    // Letter Z
    TDF_COMPRESS(1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0),    // Left square bracket
    TDF_COMPRESS(1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1),    // Backslash
    TDF_COMPRESS(0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1),    // Right square bracket
    TDF_COMPRESS(0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),    // Caret
    TDF_COMPRESS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1),    // Underscore 
    TDF_COMPRESS(1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),    // Grave accent
    TDF_COMPRESS(0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1),    // Left curly brace
    TDF_COMPRESS(0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0),    // Vertical bar
    TDF_COMPRESS(1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0),    // Right curly brace
    TDF_COMPRESS(0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0)     // Tilde 
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

td_texture *tdf_char_texture(td_i8 ch, td_rgba fg, td_rgba bg)
{
    ch = mapped_ch(ch);
    if (!IN_RANGE(ch, LOWER_LIMIT, UPPER_LIMIT))
        ch = LOWER_LIMIT;       // Space (nothing)
    const td_u8 *ch_template = texture_atlas[ch - LOWER_LIMIT];
    td_texture *out =
        tdt_create(0, 4, td_ivec2_init(CHAR_WIDTH, CHAR_HEIGHT), 0, 0);
    td_u8 *raw = tdt_get_location(td_ivec2_init(0, 0), out);
    td_u8 a[4] = TD_EXPAND_RGBA(fg), b[4] = TD_EXPAND_RGBA(bg);
    for (td_u8 i = 0; i < CHAR_PIXEL; i++) {
        td_bool is_pix = ch_template[i / 8] & (1 << (i % 8));
        for (td_u8 j = 0; j < 4; j++, raw++)
            raw[0] = is_pix ? a[j] : b[j];
    }

    return out;
}

td_texture *tdf_string_texture(const td_i8 *str, td_u32 len,
                                     td_ivec2 *s,
                                     td_rgba fg, td_rgba bg)
{
    if (!str || !len)
        return 0;
    td_u32 plen = (td_u32)len;
    td_u32 lines_count = 0, longest_line = 0;
    td_u32 *lines_length = 0;      // Filling gaps
    if (query_newline
        (str, plen, &lines_length, &lines_count, &longest_line)) {
        return 0;
    }
    // s->x is maximum character in one line, s->y is the line in the input text
    td_ivec2 texture_size = {
        (int)(longest_line * CHAR_WIDTH + calculate_pad(longest_line)),
        (int)(lines_count * CHAR_HEIGHT + calculate_pad(lines_count))
    };
    if(s) {
        *s = texture_size;
    }
    td_texture *out = tdt_create(0, 4, texture_size, 0, 0);
    if (!out) {
        free(lines_length);
        return 0;
    }
    tdt_fill(out, bg);

    // Placing character into place
    td_u32 current_index = 0;
    for (td_u32 row = 0; row < lines_count; row++) {
        td_u32 row_l = lines_length[row], start_y = row * (CHAR_HEIGHT + 1);       // 1 is pad
        for (td_u32 col = 0; col < row_l; col++) {
            td_texture *ch_texture =
                tdf_char_texture(str[current_index + col], fg, bg);
            td_u32 start_x = col * (CHAR_WIDTH + 1);
            tdt_merge(out, ch_texture, td_ivec2_init((int)start_x, (int)start_y),
                          TDT_MERGE_CROP, 1);
            tdt_free(ch_texture);
        }
        current_index += row_l; // Now at newline characater
        if (is_newline(str, &current_index, plen))
            current_index++;    // Skip newline now
    }
    free(lines_length);
    return out;
}
