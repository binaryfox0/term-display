#include "term_priv.h"

#include <windows.h>
#include <conio.h>

term_vec2 query_terminal_size()
{ 
 CONSOLE_SCREEN_BUFFER_INFO csbi;
 if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&csbi))
  return vec2_init(csbi.dwSize.X, csbi.dwSize.Y);
 return vec2_init(0, 0);
}

u8 setup_env() {}
u8 restore_env() {}

void kbpoll_events(key_callback_func func)
{
 if(_kbhit())
 {
  
 }
}