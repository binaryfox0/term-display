#ifndef TERMINAL_FONT_H
#define TERMINAL_FONT_H

#include "term_def.h"
#include "term_texture.h"

term_texture* display_char_texture(i8 ch, term_rgba color, term_rgba fg);
term_texture* display_string_texture(const i8* str, u64 len, term_ivec2* size, term_rgba color, term_rgba fg);

#endif

