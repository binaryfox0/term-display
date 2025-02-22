#include "term_priv.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

term_vec2 query_terminal_size()
{
 struct winsize ws;
 if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
  return vec2_init(ws.ws_col/2, ws.ws_row);
 return vec2_init(0, 0);
}

struct termios old, cur;
u8 disable_echo()
{
 if(tcgetattr(STDIN_FILENO, &old) == -1)
  return 1;
 cur = old;
 cur.c_lflag &= ~(ICANON | ECHO);
 if(tcsetattr(STDIN_FILENO, TCSANOW, &cur) == -1)
  return 1;
 return 0;
}

u8 restore_state()
{
 if(tcsetattr(STDIN_FILENO, TCSANOW, &old) == -1)
  return 1;
 return 0;
}
