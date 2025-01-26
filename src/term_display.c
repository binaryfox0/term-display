#include "term_display.h"
#include "term_texture.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Platform-specific header
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/* Global variable begin */
u8* display = 0; // Actual bitmap
u16 width = 0, height = 0; // The terminal size
u32 pixel_count = 0; // width * height
struct term_rgb clear_color = (struct term_rgb) { .r = 0, .g = 0, .b = 0 };
u8 numeric_options[1] = {0};
volatile u8 internal_failure = 0;
/* Global variable end */

/* Utlis function begin */
// https://stackoverflow.com/questions/23369503/get-size-of-terminal-window-rows-columns
void get_maximum_size()
{
 struct winsize ws;
 ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
 width = ws.ws_col/2;
 height = ws.ws_row;
 pixel_count = width * height;
}
void fill_display()
{
 if(!pixel_count)
  return;
 memcpy(display, &clear_color, 3);
 // Exponentially filling display
 u32 filled = 3;
 while(filled < pixel_count*3 - filled)
 {
  memcpy(&display[filled], display, filled);
  filled *= 2;
 }
 // Filling the remaining
 memcpy(&display[filled], display, (pixel_count*3 - filled));
}

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
 printf(
  "\x1b[0m"  // Reset colors mode
  "\x1b[3J"  // Clear saved line (scrollbuffer)
  "\x1b[H"   // To position 0,0
  "\x1b[2J" // Clear entire screen
 );
}

u8 set_handler(int type, void (*handler)(int))
{
 struct sigaction sa;
 sa.sa_flags = 0;
 sigemptyset(&sa.sa_mask);
 sa.sa_handler = handler;
 return (sigaction(type, &sa, 0)!=0);
}

// https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-getting-started?redirectedfrom=MSDN
struct term_vec2 ndc_to_pos(struct term_pos pos)
{
 return vec2_init(
  (u32)((pos.x + 1) * 0.5f * width),
  (u32)((1 - pos.y) * 0.5f * height)
 );
}

void transparent_blending(struct term_vec2 pos, u8 color[4])
{
 float alpha = color[3] * (1.0f / 255.0f); // To range 0-1f
 float inverse = 1.0f - alpha;
 u8* current = &display[(pos.y*width+pos.x)*3];
 current[0] = alpha * color[0] + inverse * current[0];
 current[1] = alpha * color[1] + inverse * current[1];
 current[2] = alpha * color[2] + inverse * current[2];
}

//void stdin_echo
/* Utils function end */

void resize_display(int signal)
{
 (void)signal; // Disable unused paramater warning
 get_maximum_size();
 if(
  !(display = (u8*)realloc(display, pixel_count*3))
 )
 {
  internal_failure = 1;
  return; // Uhhhh, how to continue processing without the display
 }
 texture_fill(texout_init(display, 3, vec2_init(width, height)), to_rgba(clear_color));
 clear_screen(0);
}

u8 display_init()
{
 if(!isatty(STDOUT_FILENO))
  return 1; // It's output can't seen by user (aka piped)
 resize_display(0);
 if(internal_failure)
  return 1;
 printf("\x1b[?25l"); // Hide cursor
 return 0;
/*
  set_handler(SIGWINCH, clear_screen) ||
  set_handler(SIGINT, display_free) ||
  set_handler(SIGTERM, display_free) ||
  set_handler(SIGQUIT, display_free);
*/
}

#define OPT_GET_CASE(type, value) if(get) { *(type*)option = value; return 0; }

u8 display_option(enum display_settings_types type, u8 get, void* option)
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
  OPT_GET_CASE(struct term_vec2, vec2_init(width, height));
  struct term_vec2* ptr = (struct term_vec2*)option;
  width = ptr->x;
  height = ptr->y;
  return 0;
 }
 default: break;
 }
 return 1;
}

void display_set_color(struct term_rgba color)
{
 color = pixel_blend(rgba_init(0,0,0,255), color);
 texture_fill(texout_init(display, 3, vec2_init(width, height)), color);
}

/* Sub-function begin */
u8 copy_rgb_texture(
 u8* texture,
 struct term_vec2 size,
 struct term_vec2 pos
)
{
 for(u32 row = 0; row < size.y; row++)
 {
  if(!memcpy(
   &display[((pos.y+row)*width+pos.x)*3],
   &texture[row*size.x],
   size.x*3
  ))
   return 1;
 }
 return 0;
}

u8 copy_rgba_texture(
 u8* texture,
 struct term_vec2 size,
 struct term_vec2 pos
)
{
 texture_merge(texout_init(display, 3, vec2_init(width, height)), texin_init(texture, 4, size), pos);
 
 return 0; // Just for fun
}
/* Sub-function end */

u8 display_copy_texture(
 u8* texture,
 u8 channel,
 struct term_vec2 size,
 struct term_pos pos
)
{
 struct term_vec2 display_pos = ndc_to_pos(pos);
 u32 mw = width - display_pos.x, mh = height - display_pos.y;
 // Apply thereshold
 if(size.x > mw) size.x = mw;
 if(size.y > mh) size.y = mh;
 switch(channel)
 {
 case 3: return copy_rgb_texture(texture, size, display_pos);
 case 4: return copy_rgba_texture(texture, size, display_pos);
 }
 return 1;
}

// ANSI escape sequence https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
u8 display_show()
{
 if(internal_failure || !display)
  return 1;
 sigset_t block_mask, old_mask;
 printf("\x1b[H");
 u8* ref = display;
 static u8 prev[3] = {0};
 for(u16 row = 0; row < height; row++)
 {
  for(u16 col = 0; col < width; col++, ref += 3)
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

void display_free(i32 nothing)
{
 // Show the cursor again
 // Reset color / graphics mode
 write(STDOUT_FILENO, "\x1b[?25h\x1b[0m", 11);
 clear_screen(0);
 // https://stackoverflow.com/questions/5308758/can-a-call-to-free-in-c-ever-fail
 // Technically, it can still failed, but return type is void and have undefined behaviour in the docs
 free(display);
 if(nothing)
 {
  set_handler(nothing, SIG_DFL); // Reset to default
  raise(nothing);
 }
}
