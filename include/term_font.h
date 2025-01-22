#ifndef TERMINAL_FONT_H
#define TERMINAL_FONT_H

#include "term_def.h"

u8* display_char_texture(i8 ch, struct term_rgba color, struct term_rgba fg);
u8* display_string_texture(const i8* str, u64 len, struct term_vec2* size, struct term_rgba color, struct term_rgba fg);

#endif

