#ifndef TD_FONT_H
#define TD_FONT_H

#include "td_def.h"
#include "td_texture.h"

term_texture *tdf_char_texture(term_i8 ch, term_rgba color, term_rgba fg);
term_texture *tdf_string_texture(const term_i8 * str, term_u32 len,
                                     term_ivec2 * size, term_rgba color,
                                     term_rgba fg);

#endif
