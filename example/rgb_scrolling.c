#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "term_display.h"
#include "term_font.h"

#include "example_utils.h"

#define M_PI 3.14159265358979323846

static inline term_rgb calculate_rgb(double d)
{
 return rgb_init(
   (u8)((sin(d)+1)*127.5),
   (u8)((sin(d+(2*M_PI/3))+1)*127.5),
   (u8)((sin(d+(4*M_PI/3))+1)*127.5)
 );
}

int main()
{
 u8 enable = 1;
 if(display_init())
  return 1;
 if(start_logging("statics.txt")) return 0;

 term_vec2 size = {0};
 double speed = 0.001, elapsed = 0.0;
 double delta_time = 1.0, last_log = get_time();
 while(display_is_running())
 {
  double start_frame = get_time();
  double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

  display_poll_events();

  display_set_color(to_rgba(calculate_rgb(elapsed)));

  char* string = to_string("%f", fps);
  term_texture* texture = display_string_texture(string, strlen(string), &size, rgba_init(255,255,255,255), rgba_init(0,0,0,0));
  display_copy_texture(texture, pos_init(-1.0f, 1.0f), TEXTURE_MERGE_CROP);
  texture_free(texture);

  display_show();
  elapsed += speed;

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
