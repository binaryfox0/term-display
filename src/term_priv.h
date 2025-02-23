#ifndef TERMINAL_PRIVATE_H
#define TERMINAL_PRIVATE_H

#include "term_def.h"
#include "term_display.h"

term_vec2 query_terminal_size();
u8 setup_kb();
u8 restore_kb();
void kbpoll_events(key_callback_func func);

#endif