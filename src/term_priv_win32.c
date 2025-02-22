#include "term_priv.h"

#include <Windows.h>

term_vec2 query_terminal_size()
{ 
 CONSOLE_SCREEN_BUFFER_INFO csbi;
 if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&csbi))
  return vec2_init(csbi.dwSize.X / 2, csbi.dwSize.Y);
 return vec2_init(0, 0);
}

DWORD orig_mode = 0;
u8 disable_echo()
{
 HANDLE h_in = 0;
 if((h_in = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
  return 1;
 if(!GetConsoleMode(h_in, &orig_mode))
  return 1;
 if(!SetConsoleMode(h_in, orig_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT)))
  return 1;
 return 0;
}

u8 restore_state()
{
 HANDLE h_in = 0;
 if((h_in = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
  return 1;
if(!SetConsoleMode(h_in, orig_mode))
  return 1;
 return 0;
}
