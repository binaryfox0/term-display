#ifndef TERMINAL_PRIVATE_H
#define TERMINAL_PRIVATE_H

#include "term_def.h"
#include "term_display.h"

term_vec2 query_terminal_size();
u8 setup_env(void* stop_handler);
u8 restore_env();
void kbpoll_events(key_callback_func func);

#endif