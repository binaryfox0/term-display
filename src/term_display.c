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
term_rgba default_background = (term_rgba) { .r = 0, .g = 0, .b = 0, .a = 255},
 clear_color = (term_rgba) { .r = 0, .g = 0, .b = 0, .a = 255 };
volatile u8 internal_failure = 0;
volatile u8 __display_is_running = 0;
struct {
 int x_inc, x_start, x_end;
 int y_inc, y_start, y_end;
} display_prop = { 1, 0, 0, 1, 0, 0 };
u8 numeric_options[5] =
{
 0,
 2,
 1,
 display_truecolor,
 0
};
u8 display_channel = 3;
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

static inline void query_default_background()
{
 static const char* request = "\x1b]11;?\x1b\\";
 _pwrite(STDOUT_FILENO, request, strlen(request));
 if(!timeout(100)) return;
 char buffer[32] = {0};
 if(_pread(STDIN_FILENO, buffer, 32) == -1) return;
 char r[5] = {0}, b[5] = {0}, g[5] = {0};
 if(sscanf(buffer, "\x1B]11;rgb:%4[^/]/%4[^/]/%4[^;]", r, g, b) != 3) return;
 default_background =
  rgba_init(
   strtol(r, 0, 16) / 257,
   strtol(g, 0, 16) / 257,
   strtol(b, 0, 16) / 257,
   255
 );
}
/* Utils function end */

char* display_copyright_notice()
{
 return
 "--------Terminal renderer library\n--------"
 "Copyright (c) 2025 binaryfox0 (Duy Pham Duc)\n"
 "License under the MIT License (see LICENSE file)\n\n"
 "Extremely Fast Line Algorithm Var D (Addition Fixed Point)\n"
 "Copyright 2001, By Po-Han Lin\n"
 "Freely usable in non-commercial applications as long as \n"
 "credits to Po-Han Lin and a link to http://www.edepot.com \n"
 "is provided in source code and can be seen in compiled executable.\n"
 "Commercial applications please inquire about licensing the algorithms.";
}

void resize_display()
{
 size = vec2_init(term_size.x / numeric_options[pixel_width], term_size.y / numeric_options[pixel_height]);
 if((internal_failure = texture_resize_internal(display, size)))
  return; // Uhhhh, how to continue processing without the display
 if(display_prop.y_inc < 0)
  display_prop.y_start = size.y - 1;
 else
  display_prop.y_end = size.y - 1;
 if(display_prop.x_inc < 0)
  display_prop.x_start = size.x - 1;
 else
  display_prop.x_end = size.x - 1;
 texture_fill(display, clear_color);
 clear_screen();
}

void stop_display(int signal) { (void)signal; __display_is_running = 0; }


void reload_display()
{
 if(display)
 {
  texture_free(display);
  display = 0; // Prevent free on freed memory section
 }
 if(!(display = texture_create(0, display_channel, vec2_init(1, 1), 0, 0)))
 {
  internal_failure = 1;
  return;
 }
 resize_display();
}


u8 display_init()
{
 if(!isatty(STDOUT_FILENO)) return 1;
 if(setup_env(stop_display)) return 1;
 term_size = prev_size = query_terminal_size(); // Disable calling callback on the first time
 query_default_background();
 clear_color = default_background; // Look like the background
 reload_display();
 if(internal_failure)
  return 1;
 printf("\x1b[?25l"); // Hide cursor
 __display_is_running = 1;
 return 0;
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
  if((internal_failure = texture_resize_internal(display, size)))
   return 1; // Uhhhh, how to continue processing without the display
  texture_fill(display, clear_color);
  clear_screen();
  break;
 }
 case display_type:
 {
  OPT_SET_GET(numeric_options[type], display_types);
  switch(numeric_options[type])
  {
  case display_grayscale_24:
  case display_grayscale_256:
   display_channel = 1;
   break;
  case display_truecolor_216:
  case display_truecolor:
   display_channel = 3;
   break;
  default: return 1;
  }
  reload_display();
  break;
 }
 case display_rotate:
 {
  OPT_GET_CASE(u8, numeric_options[type]);
  switch(numeric_options[type] = *(u8*)option % 4)
  {
   case 0:
    display_prop.y_inc = 1;
    display_prop.y_start = 0;
    display_prop.y_end = size.y - 1;
    break;
   case 1:
    display_prop.y_inc = -1;
    display_prop.y_start = size.y - 1;
    display_prop.y_end = -1;
    break;
   default: return 1;
  }

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
  if(numeric_options[auto_resize])
  {
   resize_display();
   private_resize_callback(size);
  }
  else clear_screen();
  prev_size = term_size;
 }
}

void display_set_key_callback(key_callback_func callback){ if(callback) private_key_callback = callback; }
void display_set_resize_callback(resize_callback_func callback) { if(callback) private_resize_callback = callback; }

void display_set_color(term_rgba color)
{
 clear_color = pixel_blend(default_background, color);
 texture_fill(display, clear_color);
}

void display_copy_texture(
 const term_texture* texture,
 const term_pos pos,
 const enum texture_merge_mode mode
)
{
 term_vec2 display_pos = ndc_to_pos(pos, size);
 texture_merge(display, texture, display_pos, mode, 0);
}

void display_draw_line(term_pos p1, term_pos p2, term_rgba color)
{
 texture_draw_line(display, ndc_to_pos(p1, size), ndc_to_pos(p2, size), color);
}

static inline u8 rgb_to_216(const u8* c)
{
 return 16 + (c[0] / 51 * 36) + (c[1] / 51 * 6) + (c[2] / 51);
}

static inline void display_cell(u8* c)
{
 if(display_channel == 1)
 {
  if(numeric_options[display_type] == display_grayscale_256) 
   printf("\x1b[48;2;%d;%d;%dm", c[0], c[0], c[0]);
  else if(numeric_options[display_type] == display_grayscale_24)
   printf("\x1b[48;5;%dm", 232 + ((c[0] * 24) >> 8));
 }
 else if(display_channel == 3)
 {
  if(numeric_options[display_type] == display_truecolor)
   printf("\x1b[48;2;%d;%d;%dm", c[0], c[1], c[2]);
  if(numeric_options[display_type] == display_truecolor_216)
   printf("\x1b[48;5;%dm", rgb_to_216(c));
 }
}

// ANSI escape sequence https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
u8 display_show()
{
 if(internal_failure || !display)
  return 1;
 printf("\x1b[H");
 u8* ptr = texture_get_location(vec2_init(0,0), display);
 static u8 prev[3] = {0};
 for(i32 row = display_prop.y_start; row != display_prop.y_end; row += display_prop.y_inc)
 {
  for(u8 i = 0; i < numeric_options[pixel_height]; i++)
  {
   u8* ref = &ptr[row * size.x * display_channel];
   for(u16 col = 0; col < size.x; col++, ref += display_channel)
   {
    if(memcmp(prev, ref, display_channel) != 0)
    {
     display_cell(ref);
     memcpy(prev, ref, display_channel);
    }
    printf("%*s", numeric_options[pixel_width], "");
   }
   printf("\x1b[1E");
  }
 }
 fflush(stdout);
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
