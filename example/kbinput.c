#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "term_display.h"
#include "term_font.h"

#include "example_utils.h"

char key_pressed[48] = "No key is pressed";

// Table
static const struct { int mod; const char* text; } mods_lookup[] =
{
 { key_ctrl, "Ctrl + " },
 { key_alt,  "Alt + " },
 { key_shift,"Shift + " }
};

static const char* fkey_name[12] =
{
 "F1",
 "F2",
 "F3",
 "F4",
 "F5",
 "F6",
 "F7",
 "F8",
 "F9",
 "F10",
 "F11",
 "F12"
};

void key_callback(int key, int mods, key_state state)
{
 memset(key_pressed, 0, 48);

 int index = 0;
 for(int i = 0; i < sizeof(mods_lookup) / sizeof(mods_lookup[0]); i++)
  if(mods & mods_lookup[i].mod)
  {
   int len = strlen(mods_lookup[i].text);
   memcpy(&key_pressed[index], mods_lookup[i].text, len);
   index += len;
  }
 if(IN_RANGE(key, term_key_astrophe, term_key_grave_accent))
 {
  key_pressed[index] = key;
  index++;
 } else if(IN_RANGE(key, term_key_f1, term_key_f12))
 {
  const char* key_name = fkey_name[key - term_key_f1];
  memcpy(&key_pressed[index], key_name, strlen(key_name));
  index += strlen(key_name);
 } else if(key == term_key_space)
 {
  memcpy(&key_pressed[index], "Space", 5);
  index += 5;
 } else if(key == term_key_backspace)
 {
  memcpy(&key_pressed[index], "Backspace", 9);
  index += 9;
 } else if(key == term_key_escape)
 {
  memcpy(&key_pressed[index], "Esc", 3);
  index += 3;
 }

 memcpy(&key_pressed[index], " is pressed.", 13);
}

int main()
{
 u8 enable = 1;
 if(display_init())
  return 1;
 display_option(auto_resize, 0, &enable);
 if(start_logging("statics.txt")) return 0;
 display_set_key_callback(key_callback);

 u64 frame_count = 0;
 term_texture* texture = 0;
 term_vec2 size = {0};
 const double program_start = get_time();
 double delta_time = 1.0, last_log = get_time();
 while(display_is_running())
 {
  frame_count++;
  double start_frame = get_time();
  double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;
 
  display_set_color(rgba_init(109, 154, 140, frame_count/7)); // Approximtely patina

  display_poll_events();

  char* string = to_string("%f", fps);
  texture = display_string_texture(string, strlen(string), &size, rgba_init(255,255,255,255), rgba_init(0,0,0,0));
  display_copy_texture(texture, pos_init(-1.0f, 1.0f), TEXTURE_MERGE_RESIZE);
  texture_free(texture);

  // The key pressed string
  texture = display_string_texture(key_pressed, strlen(key_pressed), &size, rgba_init(255,255,255,255), rgba_init(0,0,0,0));
  display_copy_texture(texture, pos_init(-1.0f, 0.0f), TEXTURE_MERGE_CROP);
  texture_free(texture);

  display_show();

  delta_time = get_time() - start_frame;
  if(start_frame - last_log >= LOG_INTERVAL)
  {
   write_log("FPS: %s", string);
   last_log = get_time();
  }
  free(string);
 }
 display_free();
 stop_logging();
 return 0;
}
