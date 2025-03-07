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
term_vec2 size = (term_vec2) { .x = 0, .y = 0 },
 term_size = (term_vec2) { .x = 0, .y = 0 }, // The terminal size
 prev_size = (term_vec2) { .x = 0, .y = 0 };
u32 pixel_count = 0; // width * height
term_rgb clear_color = (term_rgb) { .r = 0, .g = 0, .b = 0 },
 default_background = (term_rgb) { .r = 0, .g = 0, .b = 0 };
volatile u8 internal_failure = 0;
volatile u8 __display_is_running = 0;
u8 numeric_options[3] =
{
 0,
 2,
 1
};
/* Global variable end */

/* Utlis function begin */
// https://stackoverflow.com/questions/23369503/get-size-of-terminal-window-rows-columns
static inline void clear_screen()
{
 printf(
  "\x1b[0m"  // Reset colors mode
  "\x1b[3J"  // Clear saved line (scrollbuffer)
  "\x1b[H"   // To position 0,0
  "\x1b[2J" // Clear entire screen
 );
}

// https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-getting-started?redirectedfrom=MSDN
static inline term_vec2 ndc_to_pos(term_pos pos)
{
 return vec2_init(
  (u32)((pos.x + 1) * 0.5f * size.x),
  (u32)((1 - pos.y) * 0.5f * size.y)
 );
}

/* Utils function end */

void resize_display()
{
 size = vec2_init(term_size.x / numeric_options[pixel_width], term_size.y / numeric_options[pixel_height]);
 if((internal_failure = texture_resize_internal(display, size)))
  return; // Uhhhh, how to continue processing without the display
 texture_fill(display, to_rgba(clear_color));
 clear_screen();
}

void stop_display(int signal) { (void)signal; __display_is_running = 0; }

u8 display_init()
{
 if(!isatty(STDOUT_FILENO) || !(display = texture_create(0, 3, vec2_init(1,1), 0, 0)))
  return 1; // It's output can't seen by user (aka piped)
 term_size = prev_size = size = query_terminal_size(); // Disable calling callback on the first time
 resize_display();
 if(internal_failure)
  return 1;
 printf("\x1b[?25l"); // Hide cursor
 __display_is_running = 1;
 if(setup_env(stop_display)) return 1;
}

#define OPT_GET_CASE(type, value) if(get) { *(type*)option = value; return 0; }
#define OPT_SET_GET(inside, type) OPT_GET_CASE(type, (inside)); (inside) = *(type*)option
u8 display_option(display_settings_types type, u8 get, void* option)
{
 switch(type)
 {
 case auto_resize: { OPT_SET_GET(numeric_options[type], u8); break; }
 case pixel_width:
 case pixel_height:
 {
  OPT_GET_CASE(u8, numeric_options[type]);
  u8 new_value = *(u8*)option;
  numeric_options[type] = new_value;
  resize_display();
  break;
 }
 case display_size:
 {
  OPT_SET_GET(size, term_vec2);
  size = *(term_vec2*)option;
  if((internal_failure = texture_resize_internal(display, size)))
   return 1; // Uhhhh, how to continue processing without the display
  texture_fill(display, to_rgba(clear_color));
  clear_screen();
  break;
 }
 default: return 1;
 }
 return 0;
}

// Default callback
void default_key_callback(int keys, int mods, key_state state) {}
void default_resize_callback(term_vec2 new_size) {}

key_callback_func private_key_callback = default_key_callback;
resize_callback_func private_resize_callback = default_resize_callback;
void display_poll_events()
{
 kbpoll_events(private_key_callback);
 if(!compare_vec2((term_size = query_terminal_size()), prev_size))
 {
  private_resize_callback(term_size);
  if(numeric_options[auto_resize]) { size = term_size; resize_display(); }
  else clear_screen();
  prev_size = term_size;
 }
}

void display_set_key_callback(key_callback_func callback) { private_key_callback = callback; }
void display_set_resize_callback(resize_callback_func callback) { private_resize_callback = callback; }

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
 u8* ptr = texture_get_location(vec2_init(0,0), display);
 static u8 prev[3] = {0};
 for(u16 row = 0; row < size.y; row++)
 {
  for(u8 i = 0; i < numeric_options[pixel_height]; i++)
  {
   u8* ref = &ptr[row * size.x * 3];
   for(u16 col = 0; col < size.x; col++, ref += 3)
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
    printf("%*s", numeric_options[pixel_width], "");
   }
   printf("\x1b[1E");
  }
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
 restore_env();
 texture_free(display);
}
