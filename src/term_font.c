#include "term_font.h"

#include <stdlib.h>
#include <string.h>

// Macro (cuz c don't allow using const in size)
#define LOWER_LIMIT 0x20 // Exclamation mark
#define UPPER_LIMIT 0x5A // Letter Z
#define CHAR_WIDTH 3
#define CHAR_HEIGHT 5
#define CHAR_PIXEL CHAR_WIDTH * CHAR_HEIGHT

u8 texture_atlas[UPPER_LIMIT - LOWER_LIMIT + 1][CHAR_PIXEL] =
{
 {  /* Character space start */
  0, 0, 0, //
  0, 0, 0, //
  0, 0, 0, //
  0, 0, 0, //
  0, 0, 0  //
 }, /* character space end */
 {  /* Character exclamation mark start */
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 0, 0, //
  0, 1, 0  //   ##
 }, /* Character exclamation mark end */
 {  /* Character quotation mark start */
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  0, 0, 0, //
  0, 0, 0, //
  0, 0, 0  //
 }, /* Character quotation mark end */
 {  /* Character number sign start */
  1, 0, 1, // ##  ##
  1, 1, 1, // ######
  1, 0, 1, // ##  ##
  1, 1, 1, // ######
  1, 0, 1  // ##  ##
 }, /* Character number sign end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character asterisk start */
  1, 0, 1, // ##  ##
  0, 1, 0, //   ##
  1, 1, 1, // ######
  0, 1, 0, //   ##
  1, 0, 1  // ##  ##
 }, /* Character asterisk end */
 {  /* Character plus sign start */
  0, 0, 0, //
  0, 1, 0, //   ##
  1, 1, 1, // ######
  0, 1, 0, //   ##
  0, 0, 0  //
 }, /* Character plus sign end */
 {  /* Character comma start */
  0, 0, 0, //
  0, 0, 0, //
  0, 0, 0, //
  0, 1, 0, //   ##
  1, 0, 0  // ##
 }, /* Character comma end */
 {  /* Character hyphen start */
  0, 0, 0, //
  0, 0, 0, //
  1, 1, 1, // ######
  0, 0, 0, //
  0, 0, 0  //
 }, /* Character hyphen end */
 {  /* Character period start */
  0, 0, 0, //
  0, 0, 0, //
  0, 0, 0, //
  0, 0, 0, //
  0, 1, 0  //   ##
 }, /* Character period end */
 {  /* Character slash start */
  0, 0, 1, //     ##
  0, 0, 1, //     ##
  0, 1, 0, //   ##
  1, 0, 0, // ##
  1, 0, 0  // ##
 }, /* Character slash end */
 {  /* Digit zero start */
  1, 1, 1, // ######
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 1, 1  // ######
 }, /* Digit zero end */
 {  /* Digit one start */
  0, 1, 0, //   ##
  1, 1, 0, // ####
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 1, 0  //   ##
 }, /* Digit one end */
 {  /* Digit two start */
  1, 1, 0, // ####
  0, 0, 1, //     ##
  0, 1, 0, //   ##
  1, 0, 0, // ##
  1, 1, 1  // ######
 }, /* Digit two end */
 {  /* Digit three start */
  1, 1, 0, // ####
  0, 0, 1, //     ##
  0, 1, 0, //   ##
  0, 0, 1, //     ##
  1, 1, 0  // ####
 }, /* Digit three end */
 {  /* Digit four start */
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 1, 1, // ######
  0, 0, 1, //     ##
  0, 0, 1  //     ##
 }, /* Digit four end */
 {  /* Digita five start */
  1, 1, 1, // ######
  1, 0, 0, // ##
  1, 1, 0, // ####
  0, 0, 1, //     ##
  1, 1, 0  // ####
 }, /* Digit five end */
 {  /* Digit six start */
  0, 1, 1, //   ####
  1, 0, 0, // ##
  1, 1, 0, // ####
  1, 0, 1, // ##  ##
  0, 1, 0  //   ##
 }, /* Digit six end */
 {  /* Digit seven start */
  1, 1, 1, // ######
  0, 0, 1, //     ##
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 1, 0  //   ##
 }, /* Digit seven end */
 {  /* Digit eight start */
  0, 1, 0, //   ##
  1, 0, 1, // ##  ##
  0, 1, 0, //   ##
  1, 0, 1, // ##  ##
  0, 1, 0  //   ##
 }, /* Digit eight end */
 {  /* Digit nine start */
  0, 1, 0, //   ##
  1, 0, 1, // ##  ##
  0, 1, 1, //   ####
  0, 0, 1, //     ##
  1, 1, 0, // ####
 }, /* Digit nine end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Character start */
 }, /* Character end */
 {  /* Letter A start */
  0, 1, 0, //   ##
  1, 0, 1, // ##  ##
  1, 1, 1, // ######
  1, 0, 1, // ##  ##
  1, 0, 1  // ##  ##
 }, /* Letter A end */
 {  /* Letter B start */
  1, 1, 0, // ####
  1, 0, 1, // ##  ##
  1, 1, 0, // ####
  1, 0, 1, // ##  ##
  1, 1, 0  // ####
 }, /* Letter B end */
 {  /* Letter C start */
  0, 1, 0, //   ##
  1, 0, 1, // ##  ##
  1, 0, 0, // ##
  1, 0, 1, // ##  ##
  0, 1, 0  //   ##
 }, /* Letter C end */
 {  /* Letter D start */
  1, 1, 0, // ####
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 1, 0  // ####
 }, /* Letter D end */
 {  /* Letter E start */
  1, 1, 1, // ######
  1, 0, 0, // ##
  1, 1, 1, // ######
  1, 0, 0, // ##
  1, 1, 1  // ######
 }, /* Letter E end */
 {  /* Letter F start */
  1, 1, 1, // ######
  1, 0, 0, // ##
  1, 1, 1, // ######
  1, 0, 0, // ##
  1, 0, 0  // ##
 }, /* Letter F end */
 {  /* Letter G start */
  0, 1, 1, //   ####
  1, 0, 0, // ##
  1, 0, 0, // ##
  1, 0, 1, // ##  ##
  0, 1, 1  //   ####
 }, /* Letter G end */
 {  /* Letter H start */
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 1, 1, // ######
  1, 0, 1, // ##  ##
  1, 0, 1  // ##  ##
 }, /* Letter H end */
 {  /* Letter I start */
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 1, 0  //   ##
 }, /* Letter I end */
 {  /* Letter J start */
  0, 0, 1, //     ##
  0, 0, 1, //     ##
  0, 0, 1, //     ##
  1, 0, 1, // ##  ##
  0, 1, 0  //   ##
 }, /* Letter J end */
 {  /* Letter K start */
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 1, 0, // ####
  1, 0, 1, // ##  ##
  1, 0, 1  // ##  ##
 }, /* Letter K end */
 {  /* Letter L start */
  1, 0, 0, // ##
  1, 0, 0, // ##
  1, 0, 0, // ##
  1, 0, 0, // ##
  1, 1, 1  // ######
 }, /* Letter L end */
 {  /* Letter M start */
  1, 0, 1, // ##  ##
  1, 1, 1, // ######
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
 }, /* Letter M end */
 {  /* Letter N start */
  1, 1, 0, // ####
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1  // ##  ##
 }, /* Letter N end */
 {  /* Letter O start */
  0, 1, 0, //   ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  0, 1, 0  //   ##
 }, /* Letter O end */
 {  /* Letter P start */
  1, 1, 0, // ####
  1, 0, 1, // ##  ##
  1, 1, 0, // ####
  1, 0, 0, // ##
  1, 0, 0, // ##
 }, /* Letter P end */
 {  /* Letter Q start */
  0, 1, 0, //   ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 1, 1, // ######
  0, 1, 1  //   ####
 }, /* Letter Q end */
 {  /* Letter R start */
  1, 1, 0, // ####
  1, 0, 1, // ##  ##
  1, 1, 0, // ####
  1, 0, 1, // ##  ##
  1, 0, 1  // ##  ##
 }, /* Letter R end */
 {  /* Letter S start */
  0, 1, 1, //   ####
  1, 0, 0, // ##
  0, 1, 0, //   ##
  0, 0, 1, //     ##
  1, 1, 0  // ####
 }, /* Letter S end */
 {  /* Letter T start */
  1, 1, 1, // ######
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 1, 0  //   ##
 }, /* Letter T end */
 {  /* Letter U start */
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 1, 1  // ######
 }, /* Letter U end */
 {  /* Letter V start */
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  0, 1, 0  //   ##
 }, /* Letter V end */
 {  /* Letter W start */
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  1, 1, 1, // ######
  1, 0, 1  // ##  ##
 }, /* Letter W end */
 {  /* Letter X start */
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  0, 1, 0, //   ##
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
 }, /* Letter X end */
 {  /* Letter Y start */
  1, 0, 1, // ##  ##
  1, 0, 1, // ##  ##
  0, 1, 0, //   ##
  0, 1, 0, //   ##
  0, 1, 0  //   ##
 }, /* Letter Y end */
 {  /* Letter Z start */
  1, 1, 1, // ######
  0, 0, 1, //     ##
  0, 1, 0, //   ##
  1, 0, 0, // ##
  1, 1, 1  // ######
 }, /* Letter Z end */
};

/* Utils function start */
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
 if(IN_RANGE(ch, 0x61, 0x7A))
  return ch - 0x61 + 0x41; // To uppercase (how this font mapped)
 return ch;
}

/* Utils function end */

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
