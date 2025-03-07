#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "term_display.h"
#include "term_font.h"

#include "example_utils.h"

int main()
{
 u8 enable = 1;
 if(display_init())
  return 1;
 display_option(auto_resize, 0, &enable);
 if(start_logging("statics.txt")) return 0;

 term_vec2 texture_size = {0};
 term_texture* text_texture = display_string_texture("\rHello\r\nWorld!\n", -1, &texture_size, rgba_init(255,255,255,255), rgba_init(0, 0, 0, 127));
 term_vec2 size = {0};
 u64 frame_count = 0;
 double delta_time = 1.0, last_log = get_time();
 while(display_is_running())
 {
  frame_count++;
  double start_frame = get_time();
  double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

  display_poll_events();
 
  display_set_color(rgba_init(109, 154, 140, frame_count/7)); // Approximtely patina

  char* string = to_string("%f", fps);
  term_texture* texture = display_string_texture(string, strlen(string), &size, rgba_init(0,0,0,255), rgba_init(255,255,255,255));
  display_copy_texture(texture, pos_init(-1.0f, 1.0f), TEXTURE_MERGE_RESIZE);
  texture_free(texture);
  
  display_copy_texture(text_texture, pos_init(-1.0f, 0.0f), TEXTURE_MERGE_CROP);

  display_show();

  delta_time = get_time() - start_frame;
  if(start_frame - last_log >= LOG_INTERVAL)
  {
   write_log("FPS: %s", string);
   last_log = get_time();
  }
  free(string);
 }
 texture_free(text_texture);
 display_free();
 stop_logging();
 return 0;
}
