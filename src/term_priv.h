#ifndef TERMINAL_PRIVATE_H
#define TERMINAL_PRIVATE_H

#include "term_def.h"

term_vec2 query_terminal_size();
u8 disable_echo();
u8 restore_state();

#endif