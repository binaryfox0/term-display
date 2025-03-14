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

DWORD old_mode = 0;
HANDLE h_in = 0;
u8 setup_env(void* stop_handler)
{
 if((h_in = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE) return 1;
 if(!GetConsoleMode(h_in, &old_mode)) return 1;
 if(!SetConsoleMode(h_in, old_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT))) return 1;
 return
 !SetConsoleCtrlHandler((BOOL (*)(DWORD))stop_handler, 1);
}
u8 restore_env() {
 return SetConsoleMode(h_in, old_mode);
}

void kbpoll_events(key_callback_func func)
{
 if(_kbhit())
 {
  
 }
}

u8 timeout(int ms)
{
 DWORD result = WaitForSingleObject(h_in, ms);
 return result == WAIT_OBJECT_0;
}