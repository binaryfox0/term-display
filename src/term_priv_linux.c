#include "term_priv.h"
#include "term_display.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>

static struct termios old, cur;
static struct pollfd pfd;

term_vec2 query_terminal_size() {
 struct winsize ws;
 return (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) ? vec2_init(ws.ws_col, ws.ws_row) : vec2_init(0, 0);
}

u8 set_handler(int type, void (*handler)(int)) {
#ifdef _POSIX_VERSION
 struct sigaction sa = { .sa_flags = SA_SIGINFO, .sa_handler = handler };
 sigemptyset(&sa.sa_mask);
 return (sigaction(type, &sa, 0) != 0);
#else
 return signal(type, handler) == SIG_ERR;
#endif
}

u8 setup_env(void *stop_handler) {
 if (tcgetattr(STDIN_FILENO, &old) == -1) return 1;
 
 cur = old;
 cur.c_lflag &= ~(ICANON | ECHO);
 
 if (tcsetattr(STDIN_FILENO, TCSANOW, &cur) == -1) return 1;

 void (*handler)(int) = (void (*)(int))stop_handler;
 if (set_handler(SIGINT, handler) ||
  set_handler(SIGTERM, handler) ||
  set_handler(SIGQUIT, handler)) return 1;

 pfd.fd = STDIN_FILENO;
 pfd.events = POLLIN;
 
 return 0;
}

u8 restore_env() {
 if (tcsetattr(STDIN_FILENO, TCSANOW, &old) == -1) return 1;

 return (set_handler(SIGINT, SIG_DFL) ||
   set_handler(SIGTERM, SIG_DFL) ||
   set_handler(SIGQUIT, SIG_DFL));
}

#define _getch(ch) if (((ch) = getchar()) == EOF) return
#define getch_chk(val) if (getchar() != val) return

// Handle single-byte character input
static inline u8 handle_single_byte(int *ch, int *mods) {
 switch (*ch) {
  case '\0':  *ch = term_key_space; *mods |= key_ctrl; break;
  case 0x08:  *ch = term_key_backspace; *mods |= key_ctrl; break;
  case 0x09:  *ch = term_key_tab; break;
  case 0x0A:
  case 0x0D:  *ch = term_key_enter; break;
  case 0x1B:  *ch = term_key_escape; break;
  case '\"':  *ch = term_key_space; *mods |= key_shift; break;
  case ':':   *ch = term_key_semicolon; *mods |= key_shift; break;
  case '>':
  case '?':
  case '<':   *ch -= 16; *mods |= key_shift; break;
  case '+':   *ch = term_key_equal; *mods |= key_shift; break;
  case '@':   *ch = term_key_2; *mods |= key_shift; break;
  case '^':   *ch = term_key_6; *mods |= key_shift; break;
  case '_':   *ch = term_key_minus; *mods |= key_shift; break;
  case 0x7F:  *ch = term_key_backspace; break;
  default:
   if (IN_RANGE(*ch, 0x01, 0x1D)) { *ch += 64; *mods |= key_ctrl; break; }
   if (IN_RANGE(*ch, 'A', 'Z')) { *mods |= key_shift; break; }
   if (IN_RANGE(*ch, 'a', 'z')) { *ch -= 32; break; }
   if (IN_RANGE(*ch, ' ', '~')) break; // Remaining characters
   return 1;
 }
 return 0;
}

// Handle navigation keys (Arrow keys, Home, End)
static inline u8 handle_nav_key(int *ch) {
 switch (*ch) {
  case 'A': *ch = term_key_up; break;
  case 'B': *ch = term_key_down; break;
  case 'C': *ch = term_key_right; break;
  case 'D': *ch = term_key_left; break;
  case 'H': *ch = term_key_home; break;
  case 'F': *ch = term_key_end; break;
  default: return 1;
 }
 return 0;
}

static inline u8 handle_f5_below(int* ch)
{
 *ch = *ch - 'P' + term_key_f1;
 if (OUT_RANGE(*ch, term_key_f1, term_key_f4)) return 1;
 return 0;
}

static inline u8 handle_f5_above(int *ch)
{
 int first, second;
 getch_chk('[') 1;
 _getch(first) 1;
 _getch(second) 1;
 getch_chk('~') 1;
 if (first == '1') {
  switch (second) {
   case '5': *ch = term_key_f5; break;
   case '7': *ch = term_key_f6; break;
   case '8': *ch = term_key_f7; break;
   case '9': *ch = term_key_f8; break;
   default: return 1;
  }
 } else if (first == '2') {
  switch (second) {
   case '0': *ch = term_key_f9; break;
   case '1': *ch = term_key_f10; break;
   case '3': *ch = term_key_f11; break;
   case '4': *ch = term_key_f12; break;
   default: return 1;
  }
 } else return 1;
 return 0;
}

void kbpoll_events(key_callback_func func) {
 if (poll(&pfd, 1, 0) < 1) return;

 int ch = 0, mods = 0, bytes = 0;
 if (ioctl(STDIN_FILENO, FIONREAD, &bytes) == -1 || bytes == 0) return;

 _getch(ch);

 if (ch == 0x1B) {  // Escape sequence handling
  switch (bytes) {
   case 1: ch = term_key_escape; break;
   case 2: // Alt modifier
    mods |= key_alt;
    _getch(ch);
    handle_single_byte(&ch, &mods);
    break;
   case 3: // Navigation keys & F1-F4
    _getch(bytes);
    _getch(ch);
    if (bytes == '[') {
     if (handle_nav_key(&ch)) return;
    } else if (bytes == 'O') {
     if(handle_f5_below(&ch)) return;
    } else return;
    break;
   case 4: // Page Up / Page Down / Insert / Delete
    _getch(ch);
    _getch(bytes);
   // _getch(ch);
    if(ch == 'O') {
     if(bytes == '2')
     {
      _getch(ch);
      mods |= key_shift;
      if(handle_f5_below(&ch)) return;
     }
    } else if(ch == '[') {
     _getch(ch);
     if(ch == '~') {
       switch (bytes) {
        case '2': ch = term_key_insert; break;
        case '3': ch = term_key_delete; break;
        case '5': ch = term_key_page_up; break;
        case '6': ch = term_key_page_down; break;
        default: return;
       }
      }
    } else return;
    break;
   case 5: // Function keys F5 - F12
    if(handle_f5_above(&ch)) return;
    break;
   case 7:
    break;
   default: return;
  }
 } else // Single-byte characters
  if (handle_single_byte(&ch, &mods)) return;
 func(ch, mods, key_press);
}
