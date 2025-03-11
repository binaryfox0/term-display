#include <math.h> // sin
#include <string.h> // strlen
#include <stdlib.h> // free

#include "term_display.h"
#include "term_font.h"

#include "example_utils.h"

static inline double gen_rand()
{
 return -1.0 + (1.0 - -1.0) * ((double)rand() / RAND_MAX);
}

int main()
{
 u8 enable = 1;
 if(display_init() || start_logging("statics.txt"))
  return 1;

 term_vec2 size = {0};
 double delta_time = 1.0, last_log = get_time();
 const double max_dt = 1.0 / 2;
 while(display_is_running())
 {
  double start_frame = get_time();
  double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

  display_poll_events();

  display_set_color(rgba_init(0,0,0,255));

  char* string = to_string("%f", fps);
  term_texture* texture = display_string_texture(string, strlen(string), &size, rgba_init(255,255,255,255), rgba_init(0,0,0,0));
  display_copy_texture(texture, pos_init(-1.0f, 1.0f), TEXTURE_MERGE_CROP);
  texture_free(texture);

  term_pos p1 = pos_init(gen_rand(), gen_rand()), p2 = pos_init(gen_rand(), gen_rand()), p3 = pos_init(gen_rand(), gen_rand());
  display_draw_line(p1, p2, rgba_init(255,255,255,255));
  display_draw_line(p2, p3, rgba_init(255,255,255,255));
  display_draw_line(p3, p1, rgba_init(255,255,255,255));

  display_show();

  while((delta_time = get_time() - start_frame) < max_dt){}
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
