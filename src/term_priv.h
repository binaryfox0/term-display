#ifndef TERMINAL_PRIVATE_H
#define TERMINAL_PRIVATE_H

#include "term_def.h"
#include "term_display.h"

#ifdef TERMINAL_WINDOWS
 #define STDIN_FILENO _fileno(stdin)

 #define _pread _read
 #define _pwrite _write
#endif

#ifdef TERMINAL_UNIX
 #define _pread read
 #define _pwrite write
#endif

term_vec2 query_terminal_size();
u8 setup_env(void* stop_handler);
u8 restore_env();
void kbpoll_events(key_callback_func func);
u8 timeout(int ms);

#endif