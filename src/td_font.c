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

#include "td_priv.h"
#include "td_dynarr.h"
#include "td_texture.h"

// Macro (cuz c don't allow using const in size)
#define ATLAS_SIZE 69
#define LOWER_LIMIT ' '
#define UPPER_LIMIT '~'
#define CHAR_WIDTH 3
#define CHAR_HEIGHT 5
#define CHAR_PIXEL CHAR_WIDTH * CHAR_HEIGHT

#define TDF_ARRSZ(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define TDF_COMPRESS(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
        (a << 0) | (b << 1) | (c << 2) | (d << 3) | \
        (e << 4) | (f << 5) | (g << 6) | (h << 7) | \
        (i << 8) | (j << 9) | (k << 10) | (l << 11) | \
        (m << 12) | (n << 13) | (o << 14)

typedef struct tdp_map {
    int s1, s2; // source start, source end
    int d; // destination
} tdp_map;

typedef struct td_font {
    td_i32 glyph_height; // auto update when insert chatacters
    td_i32 max_height_count;

    tdp_dynarr characters; // td_dynarr* -> td_texture**
    tdp_dynarr ranges; // td_ivec2
    tdp_dynarr mapper; // tdp_map
} td_font;

typedef struct tdp_font_template {
    td_u16 *template;
    td_ivec2 range;
} tdp_font_template;

static const tdp_font_template tdp_template_no1 = {
    .template = (td_u16[])    
    {
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
    },
    .range = { .x = ' ', .y = '`' }
};

static const tdp_font_template tdp_template_no2 = {
    .template = (td_u16[])
    {
        TDF_COMPRESS(0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1),    // Left curly brace
        TDF_COMPRESS(0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0),    // Vertical bar
        TDF_COMPRESS(1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0),    // Right curly brace
        TDF_COMPRESS(0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0)     // Tilde 
    },
    .range = { .x = '{', .y = '~' }
};

td_texture* tdp_codepoint_resolve(
    const td_font* font, const int character)
{
    int ch = character;
    for(size_t i = 0; i < font->mapper.used; i++)
    {
        tdp_map map = ((tdp_map*)font->mapper.ptr)[i];
        if(ch < map.s1 || ch > map.s2)
            continue;
        ch = ch - map.s1 + map.d;
    }
    
    for(size_t i = 0; i < font->ranges.used; i++)
    {
        td_ivec2 range = ((td_ivec2*)font->ranges.ptr)[i];
        if(ch < range.x || ch > range.y)
            continue;
        tdp_dynarr chs = tdp_dynarr_get(&font->characters, tdp_dynarr, i); 
        return tdp_dynarr_get(&chs, td_texture*, ch - range.x);
    }
    return 0;
}

TD_INLINE td_bool tdp_is_newline(const char *str, const td_u64 str_len, td_u64 *current)
{
    td_u8 cr = 0;                  // CR LF detection
    if (str[*current] == '\n' || (cr = (str[*current] == '\r'))) {
        if (cr && *current + 1 < str_len && str[*current + 1] == '\n')
            (*current)++;
        return td_true;
    }
    return td_false;
}

#define TDP_FONT_SPACING 1
td_ivec2 td_calc_text_size(
        const td_font *font, const char *str, const td_u64 str_len)
{
    if(!font || !str || !str_len)
        return (td_ivec2){0};

    td_i32 curr_len = -TDP_FONT_SPACING, max_len = 0;
    td_i32 lines = 0;

    for(td_u64 i = 0; i < str_len && str[i]; i++)
    {
        if(tdp_is_newline(str, str_len, &i))
        {
            max_len = tdp_max(curr_len, max_len);
            curr_len = -TDP_FONT_SPACING;
            lines++;
            continue;
        }

        td_texture *glyph_tex = tdp_codepoint_resolve(font, str[i]);
        if(!glyph_tex)
            continue;

        curr_len += glyph_tex->size.x + TDP_FONT_SPACING;
    }

    if(curr_len > 0) {
        max_len = tdp_max(curr_len, max_len);
        lines++;
    }

    return (td_ivec2){
        .x = max_len,
        .y = lines * font->glyph_height + (lines - 1) * TDP_FONT_SPACING
    };
}


td_font* td_font_create(void)
{
    td_font* out = malloc(sizeof(td_font));
    if(!out)
        return 0;
    out->characters = (tdp_dynarr) { 0, 0, sizeof(tdp_dynarr), 0 };
    out->ranges = (tdp_dynarr) { 0, 0, sizeof(td_ivec2), 0 };
    out->mapper = (tdp_dynarr) { 0, 0, sizeof(tdp_map), 0 };
    return out;
}

static void tdp_append_textures_from_template(
        td_font *font, const tdp_font_template *template,
        const td_rgba background, const td_rgba foreground
)
{
    if(!font || !template)
        return;
    const td_ivec2 range = template->range;
    if(!tdp_dynarr_add(&font->ranges, &template->range))
        return;
        
    const td_u8 a[4] = TD_EXPAND_RGBA(foreground),
                b[4] = TD_EXPAND_RGBA(background);
    
    size_t chars_count = (size_t)(range.y - range.x + 1);
    tdp_dynarr chars_range = {0};
    if(!tdp_dynarr_new(&chars_range, chars_count, sizeof(td_texture*)))
        goto fail;

    for(size_t j = 0; j < chars_count; j++)
    { 
        td_texture *char_tex = td_texture_create(
            0, 4, (td_ivec2){CHAR_WIDTH, CHAR_HEIGHT}, 0, 0);
        if(!char_tex) 
            continue;

        td_u8 *raw = td_texture_get_pixel(char_tex, (td_ivec2){0});
        const td_u16 char_template = template->template[j];

        int bitmask = 1;
        for (td_u8 k = 0; k < CHAR_PIXEL; k++, bitmask <<= 1) {
            td_bool is_pix = char_template & bitmask;
            for (td_u8 l = 0; l < 4; l++, raw++)
                *raw = is_pix ? a[l] : b[l];
        }
        tdp_dynarr_add(&chars_range, &char_tex);
    }

    if(!tdp_dynarr_add(&font->characters, &chars_range))
        goto fail;

    return;

fail:
    for(td_u64 i = 0; i < chars_range.used; i++)
        td_texture_destroy(tdp_dynarr_get(&chars_range, td_texture*, i));
    font->ranges.used--;
}

td_font* td_default_font(const td_rgba foreground, const td_rgba background)
{
    td_font* out = td_font_create();
    if(!out)
        return 0;

    tdp_append_textures_from_template(out, &tdp_template_no1, background, foreground);
    tdp_append_textures_from_template(out, &tdp_template_no2, background, foreground);
   
    out->glyph_height = CHAR_HEIGHT;

    // Lowercase to uppercase
    tdp_dynarr_add(&out->mapper, (tdp_map[1]){{ 'a', 'z', 'A'}});
    
    return out;
}

void td_destroy_font(td_font* font)
{
    for(size_t i = 0; i < font->characters.used; i++)
    {
        tdp_dynarr chars_range = tdp_dynarr_get(&font->characters, tdp_dynarr, i);
        for(size_t j = 0; j < chars_range.used; j++)
        {
            td_texture *tex = tdp_dynarr_get(&chars_range, td_texture*, j);
            if(!tex)
                continue;
            td_texture_destroy(tex);
        }
        free(chars_range.ptr);
    }
    free(font->characters.ptr);
    free(font->ranges.ptr);
    free(font->mapper.ptr);
    free(font);
}

td_u64 tdp_find_character_range(
        const td_font *font,
        const td_i32 codepoint,
        td_u64 *lower_bound
)
{
    const td_ivec2 *ranges = (const td_ivec2 *)font->ranges.ptr;
    td_u64 count = font->ranges.used;

    td_u64 lo = 0;
    td_u64 hi = count;

    while (lo < hi) {
        td_u64 mid = lo + (hi - lo) / 2;
        td_ivec2 r = ranges[mid];

        if (codepoint < r.x) {
            hi = mid;
        }
        else if (codepoint > r.y) {
            lo = mid + 1;
        }
        else {
            // inside [x, y]
            return mid;
        }
    }

    if(lower_bound)
        *lower_bound = lo;

    return count; // idx == end (not found)
}

td_err td_font_replace_char(
        td_font *font,
        const td_i32 codepoint,
        const td_texture *tex
)
{
    if(!font || !tex)
        return TD_ERR_INVALID_ARG;

    td_u64 ranges_end = font->ranges.used;
    td_texture *dup_tex = td_texture_copy(tex);
    if(!dup_tex)
        return TD_ERR_OUT_OF_MEMORY;

    // Case 1: Already existed characters range
    td_u64 idx = tdp_find_character_range(font, codepoint, 0);
    if(idx != ranges_end)
    {
        if(idx >= font->characters.used)
            ;
        td_ivec2 range = tdp_dynarr_get(&font->ranges, td_ivec2, idx);
        tdp_dynarr *chars_range = &tdp_dynarr_get(&font->characters, tdp_dynarr, idx);
        td_u64 char_idx = (td_u64)(codepoint - range.x);
        if(char_idx >= chars_range->used)
            ;
        td_texture **char_tex = &tdp_dynarr_get(chars_range, td_texture*, char_idx);
        td_i32 old_texh = (*char_tex)->size.y;
        if(*char_tex)
            td_texture_destroy(*char_tex);
        *char_tex = dup_tex;

        if(dup_tex->size.y > old_texh)
        {
            font->glyph_height = dup_tex->size.y;
            font->max_height_count = 0;
        }

        return TD_ERR_OK;
    }

    // Case 2: Find the nearest lower consecutive characters range
    td_u64 lower_bound = 0;
    idx = tdp_find_character_range(font, codepoint > 0 ? codepoint - 1 : codepoint, &lower_bound);
    if(idx != ranges_end)
    {
        // TODO: add a case to find the nearest upper consecutive characters range
        // then merge into lower range
        td_ivec2 *range = &tdp_dynarr_get(&font->ranges, td_ivec2, idx);
        tdp_dynarr *chars_range = &tdp_dynarr_get(&font->characters, tdp_dynarr, idx);
        td_u64 char_idx = (td_u64)(codepoint - range->x);
        if(char_idx >= chars_range->used + 1)
            ;
        if(!tdp_dynarr_add(chars_range, dup_tex))
            return TD_ERR_OUT_OF_MEMORY;
        range->y++;
        return TD_ERR_OK;
    }

    // Case 3: No existing range nearby
    td_ivec2 range = { codepoint, codepoint };
    tdp_dynarr chars_range = {0};
    if(!tdp_dynarr_new(&chars_range, 4, sizeof(td_texture*)))
        return TD_ERR_OUT_OF_MEMORY;
    tdp_dynarr_add(&chars_range, &dup_tex);
    if(
        !tdp_dynarr_insert(&font->ranges, lower_bound, &range) ||
        !tdp_dynarr_insert(&font->characters, lower_bound, &chars_range)
      ) return TD_ERR_OUT_OF_MEMORY;

    return TD_ERR_OK;
}

td_texture *td_render_char(const td_font* font, const td_i32 character)
{
    return tdp_codepoint_resolve(font, character);
}

td_err td_render_string_into(
        const td_font *font,
        const td_ivec2 pos,
        const char *str,
        const td_u64 str_len,
        td_texture *tex_out
)
{
    if(!font || !str || !tex_out || pos.x < 0 || pos.y < 0)
        return TD_ERR_INVALID_ARG;
    if(!str_len || pos.x >= tex_out->size.x || pos.y >= tex_out->size.y)
        return TD_ERR_OK;

    int line_y = pos.y;
    int char_x = pos.x;
    for(td_u64 i = 0; i < str_len && str[i]; i++)
    {
        if(tdp_is_newline(str, str_len, &i))
        {
            line_y += font->glyph_height + 1;
            if(line_y >= tex_out->size.y)
                break;
            char_x = pos.x;
            continue;
        }

        if(char_x >= tex_out->size.x)
            continue;
        
        td_texture *glyph_tex = tdp_codepoint_resolve(font, str[i]);
        if(!glyph_tex)
            continue;
        td_texture_merge(tex_out, glyph_tex, (td_ivec2){char_x, line_y}, td_false);
        char_x += glyph_tex->size.x + TDP_FONT_SPACING;
    }
    return TD_ERR_OK;
}

td_texture *td_render_string(
        const td_font* font,
        const char *str, 
        const td_u64 str_len
)
{
    if (!font || !str || !str_len)
        return 0;

    td_ivec2 textsz = td_calc_text_size(font, str, str_len);
    if(!textsz.x || !textsz.y)
        return 0;
    td_texture *tex = td_texture_create(0, 4, textsz, td_false, td_false);
    if(!tex)
        return 0;
    td_render_string_into(font, (td_ivec2){0}, str, str_len, tex);
    return tex;
}

