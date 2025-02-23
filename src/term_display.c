#include "term_display.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Platform-specific header
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "term_priv.h"

/* Global variable begin */
term_texture* display = 0;
term_vec2 term_size = (term_vec2) { .x = 0, .y = 0 }; // The terminal size
u32 pixel_count = 0; // width * height
term_rgb clear_color = (term_rgb) { .r = 0, .g = 0, .b = 0 };
u8 numeric_options[1] = {0};
volatile u8 internal_failure = 0;
volatile u8 __display_is_running = 0;
/* Global variable end */

/* Utlis function begin */



// https://stackoverflow.com/questions/23369503/get-size-of-terminal-window-rows-columns
void clear_screen(int nothing)
{
 (void)nothing; // Just for plugging into sa_handler
 write(
  STDOUT_FILENO,
  "\x1b[0m"  // Reset colors mode
  "\x1b[3J"  // Clear saved line (scrollbuffer)
  "\x1b[H"   // To position 0,0
  "\x1b[2J", // Clear entire screen
  16
 );
}

u8 set_handler(int type, void (*handler)(int))
{
 struct sigaction sa;
 sa.sa_flags = SA_SIGINFO;
 sigemptyset(&sa.sa_mask);
 sa.sa_handler = handler;
 return (sigaction(type, &sa, 0)!=0);
}

// https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-getting-started?redirectedfrom=MSDN
static inline term_vec2 ndc_to_pos(term_pos pos)
{
 return vec2_init(
  (u32)((pos.x + 1) * 0.5f * term_size.x),
  (u32)((1 - pos.y) * 0.5f * term_size.y)
 );
}

/* Utils function end */

void resize_display(int signal)
{
 (void)signal; // Disable unused paramater warning
 term_size = query_terminal_size();
 if((internal_failure = texture_resize_internal(display, term_size)))
  return; // Uhhhh, how to continue processing without the display
 texture_fill(display, to_rgba(clear_color));
 clear_screen(0);
}

void stop_display(int signal) { (void)signal; __display_is_running = 0; }

u8 display_init()
{
 if(!isatty(STDOUT_FILENO) || !(display = texture_create(0, 3, vec2_init(1,1), 0, 0)))
  return 1; // It's output can't seen by user (aka piped)
 resize_display(0);
 if(internal_failure)
  return 1;
 printf("\x1b[?25l"); // Hide cursor
 __display_is_running = 1;
#ifdef TERMINAL_WINDOWS
#else
 if(setup_kb()) return 1;
#endif
 return
  set_handler(SIGWINCH, clear_screen) || // Remove resizing artifact
  set_handler(SIGINT, stop_display) ||
  set_handler(SIGTERM, stop_display) ||
  set_handler(SIGQUIT, stop_display);
}

#define OPT_GET_CASE(type, value) if(get) { *(type*)option = value; return 0; }

u8 display_option(display_settings_types type, u8 get, void* option)
{
 switch(type)
 {
 case auto_resize:
 {
  OPT_GET_CASE(u8, numeric_options[type]);
  return set_handler(
   SIGWINCH,
   (numeric_options[type] = *(u8*)option) ?
    resize_display :
    SIG_DFL
  );
 }
 case display_size:
 {
  OPT_GET_CASE(term_vec2, term_size);
  term_size = *(term_vec2*)option;
  return 0;
 }
 default: break;
 }
 return 1;
}

key_callback_func private_key_callback = 0;
void display_poll_events()
{
 kbpoll_events(private_key_callback);
}

void display_set_key_callback(key_callback_func callback)
{
#ifdef TERMINAL_WINDOWS
#else
 private_key_callback = callback;
#endif
}

void display_set_color(term_rgba color)
{
 color = pixel_blend(rgba_init(0,0,0,255), color);
 texture_fill(display, color);
}

void display_copy_texture(
 const term_texture* texture,
 const term_pos pos,
 const enum texture_merge_mode mode
)
{
 term_vec2 display_pos = ndc_to_pos(pos);
 texture_merge(display, texture, display_pos, mode, 0);
}

// ANSI escape sequence https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
u8 display_show()
{
 if(internal_failure || !display)
  return 1;
 printf("\x1b[H");
 u8* ref = texture_get_location(vec2_init(0,0), display);
 static u8 prev[3] = {0};
 for(u16 row = 0; row < term_size.y; row++)
 {
  for(u16 col = 0; col < term_size.x; col++, ref += 3)
  {
   if(
    prev[0] != ref[0] ||
    prev[1] != ref[1] ||
    prev[2] != ref[2]
   )
   {
    printf("\x1b[48;2;%d;%d;%dm",
     ref[0],
     ref[1],
     ref[2]
    );
    memcpy(prev, ref, 3);
   }
   printf("  ");
  }
  printf("\x1b[1E");
 }
 return 0;
}

void display_free()
{
 // Show the cursor again
 // Reset color / graphics mode
 write(STDOUT_FILENO, "\x1b[?25h\x1b[0m", 11);
 fflush(stdout); // Flush remaining data
 clear_screen(0);
 restore_state();
 // https://stackoverflow.com/questions/5308758/can-a-call-to-free-in-c-ever-fail
 // Technically, it can still failed, but return type is void and have undefined behaviour in the docs
 texture_free(display);
}
