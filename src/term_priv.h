#ifndef TERMINAL_PRIVATE_H
#define TERMINAL_PRIVATE_H

#include "term_def.h"
#include "term_display.h"

#ifdef TERMINAL_WINDOWS
 #include <io.h>
 #ifndef STDIN_FILENO
  #define STDIN_FILENO _fileno(stdin)
 #endif

 #ifndef STDOUT_FILENO
  #define STDOUT_FILENO _fileno(stdout)
 #endif

 #define _pread _read
 #define _pwrite _write
 #define _pisatty _isatty
#endif

#ifdef TERMINAL_UNIX
 #include <unistd.h>
 #define _pread read
 #define _pwrite write
 #define _pisatty isatty
#endif

term_ivec2 query_terminal_size();
u8 setup_env(void* stop_handler);
u8 restore_env();
void kbpoll_events(key_callback_func func);
u8 timeout(int ms);

#endif