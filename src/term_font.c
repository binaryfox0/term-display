#include "term_font.h"

#include <stdlib.h>
#include <string.h>

// Macro (cuz c don't allow using const in size)
#define ATLAS_SIZE 69
#define LOWER_LIMIT ' '
#define UPPER_LIMIT '~'
#define CHAR_WIDTH 3
#define CHAR_HEIGHT 5
#define CHAR_PIXEL CHAR_WIDTH * CHAR_HEIGHT

u8 texture_atlas[ATLAS_SIZE][CHAR_PIXEL] =
{
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Space
 { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0 }, // Exclamation mark
 { 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Quotation mark
 { 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1 }, // Number sign
 { 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0 }, // Dollar sign
 { 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1 }, // Percent sign
 { 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1 }, // Ampersand
 { 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Astrophobe
 { 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0 }, // Left parenthesis
 { 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0 }, // Right parenthesis
 { 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1 }, // Asterisk
 { 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0 }, // Plus sign
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0 }, // Comma
 { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 }, // Hyphen
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 }, // Period
 { 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0 }, // Slash
 { 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1 }, // Digit 0
 { 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, // Digit 1
 { 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1 }, // Digit 2
 { 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0 }, // Digit 3
 { 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1 }, // Digit 4
 { 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0 }, // Digit 5
 { 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0 }, // Digit 6
 { 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, // Digit 7
 { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 }, // Digit 8
 { 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0 }, // Digit 9
 { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 }, // Colon
 { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0 }, // Semicolon
 { 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 }, // Less-than
 { 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0 }, // Equals-to
 { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0 }, // Greater-than
 { 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0 }, // Question mark
 { 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1 }, // At sign
 { 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1 }, // Letter A
 { 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0 }, // Letter B
 { 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0 }, // Letter C
 { 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0 }, // Letter D
 { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1 }, // Letter E
 { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0 }, // Letter F
 { 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1 }, // Letter G
 { 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1 }, // Letter H
 { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, // Letter I
 { 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0 }, // Letter J
 { 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1 }, // Letter K
 { 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1 }, // Letter L
 { 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1 }, // Letter M
 { 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1 }, // Letter N
 { 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0 }, // Letter O
 { 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0 }, // Letter P
 { 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1 }, // Letter Q
 { 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1 }, // Letter R
 { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 }, // Letter S
 { 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, // Letter T
 { 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1 }, // Letter U
 { 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0 }, // Letter V
 { 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1 }, // Letter W
 { 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1 }, // Letter X
 { 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 }, // Letter Y
 { 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1 }, // Letter Z
 { 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0 }, // Left square bracket
 { 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1 }, // Backslash
 { 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1 }, // Right square bracket
 { 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Caret
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 }, // Underscore 
 { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Grave accent
 { 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1 }, // Left curly brace
 { 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0 }, // Vertical bar
 { 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0 }, // Right curly brace
 { 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  // Tilde 
};

u8 calculate_pad(u8 ch)
{
 return (ch ? ch - 1 : 0);
}

u8 is_newline(const i8* str, u64* current, u64 max)
{
 u8 cr = 0; // CR LF detection
 if(str[*current] == '\n' || (cr = (str[*current] == '\r')))
 {
  if(cr && *current + 1 < max && str[*current + 1] == '\n')
   (*current)++;
  return 1;
 }
 return 0;
}

u8 query_newline(
 const i8* str,
 u64 len,
 u64** lines_length, // Lines length info
 u32* line_count,
 u32* longest_line
)
{
 static const u32 min_realloc = 10;

 u64 line_len = 0, current_size = 0;
 *line_count = 0;
 *longest_line = 0;
 for(u64 i = 0; i < len && str[i]; i++)
 {
  if(is_newline(str, &i, len))
  {
   if(current_size <= *line_count)
   {
    if(!(*lines_length = (u64*)realloc(
     *lines_length,
     (current_size = current_size + min_realloc)*sizeof(u64)))
    )
     return 1;
   }
   *longest_line = *longest_line < line_len ? line_len : *longest_line;
   (*lines_length)[*line_count] = line_len;
   line_len = 0;
   (*line_count)++;
   continue;
  }
  line_len++;
 }
 // Final line without the newline character
 if(line_len > 0)
 {
  if(current_size == *line_count) // cuz in the loop it have been alocated enough but not for this
  {
   if(!(*lines_length = (u64*)realloc(*lines_length, (current_size + 1)*sizeof(u64))))
    return 1;
  }
  (*lines_length)[*line_count] = line_len;
  *longest_line = (*longest_line < line_len) ? line_len : *longest_line;
  (*line_count)++;
 }
 // Remove unused part
 if(*line_count < current_size)
 {
  if(!(*lines_length = (u64*)realloc(*lines_length, *line_count*sizeof(u64))))
   return 1;
 }
 return 0;
}

i8 mapped_ch(i8 ch)
{
 if(IN_RANGE(ch, 'a', '~'))
  return ch - 'a' + 'A'; // To uppercase (how this font mapped)
 return ch;
}

term_texture* display_char_texture(i8 ch, term_rgba color, term_rgba fg)
{
 ch = mapped_ch(ch);
 if(!IN_RANGE(ch, LOWER_LIMIT, UPPER_LIMIT))
  ch = LOWER_LIMIT; // Space (nothing)
 u8* ch_template = texture_atlas[ch - LOWER_LIMIT];
 term_texture* out = texture_create(0, 4, vec2_init(CHAR_WIDTH, CHAR_HEIGHT), 0, 0);
 u8* raw = texture_get_location(vec2_init(0, 0), out);
 u8 a[4] = EXPAND_RGBA(color), b[4] = EXPAND_RGBA(fg);
 for(u8 i = 0; i < CHAR_PIXEL; i++)
  for(u8 j = 0; j < 4; j++, raw++)
   raw[0] = ch_template[i] ? a[j] : b[j];
   
 return out;
}

term_texture* display_string_texture(
 const i8* str, u64 len,
 term_vec2* s,
 term_rgba color,
 term_rgba fg
)
{
 u32 lines_count = 0, longest_line = 0;
 u64* lines_length = 0; // Filling gaps
 query_newline(str, len, &lines_length, &lines_count, &longest_line);
 // s->x is maximum character in one line, s->y is the line in the input text
 s->x = longest_line * CHAR_WIDTH + calculate_pad(longest_line);
 s->y = lines_count * CHAR_HEIGHT + calculate_pad(lines_count);
 term_texture* out = texture_create(0, 4, *s, 0, 0);
 if(!out) return 0;
 texture_fill(out, fg);

 // Placing character into place
 u64 current_index = 0;
 for(u32 row = 0; row < lines_count; row++)
 {
  u64 row_l = lines_length[row],
   start_y = row * (CHAR_HEIGHT + 1); // 1 is pad
  for(u64 col = 0; col < row_l; col++)
  {
   term_texture* ch_texture = display_char_texture(str[current_index+col], color, fg);
   u64 start_x = col * (CHAR_WIDTH + 1);
   texture_merge(out, ch_texture, vec2_init(start_x, start_y), TEXTURE_MERGE_CROP, 1);
   texture_free(ch_texture);
  }
  current_index += row_l; // Now at newline characater
  if(is_newline(str, &current_index, len))
   current_index++; // Skip newline now
 }
 free(lines_length);
 return out;
}
