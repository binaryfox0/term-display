#include "term_priv.h"
#include "term_display.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>

term_vec2 query_terminal_size()
{
 struct winsize ws;
 if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
  return vec2_init(ws.ws_col/2, ws.ws_row);
 return vec2_init(0, 0);
}

struct termios old, cur;
struct pollfd pfd;
u8 setup_kb()
{
 if(tcgetattr(STDIN_FILENO, &old) == -1)
  return 1;
 cur = old;
 cur.c_lflag &= ~(ICANON | ECHO);
 if(tcsetattr(STDIN_FILENO, TCSANOW, &cur) == -1)
  return 1;
 pfd.fd = STDIN_FILENO;
 pfd.events = POLLIN;
 return 0;
}

u8 restore_kb()
{
 if(tcsetattr(STDIN_FILENO, TCSANOW, &old) == -1)
  return 1;
 return 0;
}

void kbpoll_events(key_callback_func func)
{
 if(poll(&pfd, 1, 0) > 0)
 {
  u8 ch = 0;
  int mods = 0, bytes = 0;
  if(ioctl(STDIN_FILENO, FIONREAD, &bytes) == -1 || bytes == 0)
   return;
  if(read(STDIN_FILENO, &ch, 1) == -1)
   return;
 // if(ch == 0x1b)
  if(IN_RANGE(ch, 0x01, 0x1A)) // Ctrl + <key>
  {
   ch += 64;
   mods |= key_ctrl;
  }
  else if(IN_RANGE(ch, 0x41, 0x5A)) // Uppercase character
   mods |= key_shift;
  else if(IN_RANGE(ch, 0x61, 0x7A)) ch -= 32;
  func(ch, mods, key_press);
 }
}