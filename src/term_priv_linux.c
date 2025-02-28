#include "term_priv.h"
#include "term_display.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <stdio.h>

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

static inline u8 handle_single_byte(int *ch, int *mods)
{
 switch(*ch)
 {
  case '\0': { *ch = term_key_grave_accent; *mods |= key_ctrl; break; }
  case '!':
  case '#':
  case '$':
  case '%':
  {
   *ch += 16;
   *mods |= key_shift;
   break;
  }
  case '+': { *ch = term_key_equal; *mods |= key_shift; break;}
  case '@': { *ch = term_key_2; *mods |= key_shift; break; }
  case '^': { *ch = term_key_6; *mods |= key_shift; break; }
  case '&':
  case '(': { *ch += 17; *mods |= key_shift; break; }
  case '*': { *ch = term_key_8; *mods |= key_shift; break; }
  case ')': { *ch = term_key_0; *mods |= key_shift; break; }
  case '{':
  case '|':
  case '}': { *ch -= 32; *mods |=key_shift; break; }
  case 0x7f: { *ch = term_key_backspace; break; }
  default:
  {
   if(IN_RANGE(*ch, 0x01, 0x1D)) { *ch += 64; *mods |= key_ctrl; break; }
   if(IN_RANGE(*ch, 'A', 'Z')) { *mods |= key_shift; break; }
   if(IN_RANGE(*ch, 'a', 'z')) { *ch -= 32; break; }
   if(IN_RANGE(*ch, ' ', '~')) break; // Remaining characters

   return 1;
  }
 }
 return 0;
}

// Arrow key + Home/End
static inline u8 handle_nav_key(int *ch)
{
 switch(*ch)
 {
  case 'A': { *ch = term_key_up; break; }
  case 'B': { *ch = term_key_down; break; }
  case 'C': { *ch = term_key_right; break; }
  case 'D': { *ch = term_key_left; break; }
  case 'H': { *ch = term_key_home; break; }
  case 'F': { *ch = term_key_end; break; }
  default: return 1;
 }
 return 0;
}

#define _getch(ch) if(((ch) = getchar()) == EOF) return
#define getch_chk(val) if(getchar() != val) return
void kbpoll_events(key_callback_func func)
{
 if(poll(&pfd, 1, 0) < 1) return;

 int ch = 0;
 int mods = 0, bytes = 0;
 if(ioctl(STDIN_FILENO, FIONREAD, &bytes) == -1 || bytes == 0)
  return;
 _getch(ch);
 switch(ch)
 {
  case 0x0d: { ch = term_key_enter; break; }
  case 0x1b:
  {
   switch(bytes)
   {
    case 1: { ch = term_key_escape; break; }
    case 2: // Alt
    {
     mods |= key_alt;
     _getch(ch);
     handle_single_byte(&ch, &mods);
     break;
    }
    case 3:
    {
     getch_chk(0x5b);
     _getch(ch);
     if(handle_nav_key(&ch)) return;
     break;
    }
    case 4: // PGUP/PGDN
    {
     getch_chk('[');
     _getch(bytes);
     getch_chk('~');
     switch(bytes)
     {
      case '2': { ch = term_key_insert; break; }
      case '3': { ch = term_key_delete; break; }
      case '5': { ch = term_key_page_up; break; }
      case '6': { ch = term_key_page_down; break; }
      default: return;
     }
     break;
    }
    case 5: // F5 to F12
    {
     getch_chk('[');
     _getch(ch);
     _getch(bytes);
     getch_chk('~');
     if(ch == '1')
     {
      switch(bytes)
      {
       case '5': { ch = term_key_f5; break; }
       case '7': { ch = term_key_f6; break; }
       case '8': { ch = term_key_f7; break; }
       case '9': { ch = term_key_f8; break; }
       default: return;
      }
     }
     else if(ch == '2')
     {
      switch(bytes)
      {
       case '0': { ch = term_key_f9; break; }
       case '1': { ch = term_key_f10; break; }
       case '3': { ch = term_key_f11; break; }
       case '4': { ch = term_key_f12; break; }
       default: return;
      }
     } else return;
     break;
    }
    default: return;
   }
   break;
  }
  case 0x7f: { ch = term_key_backspace; break; }
  default:
  {
   bytes = ch;
   if(handle_single_byte(&ch, &mods)) return;
   break;
  }
 }
 func(ch, mods, key_press);
}
