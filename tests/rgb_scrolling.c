#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "term_display.h"
#include "term_font.h"

#include "test_utils.h"

#define M_PI 3.14159265358979323846

term_rgb calculate_rgb(double d)
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
// display_option(auto_resize, 0, &enable);
 FILE* statics = fopen("statics.txt", "w");
 if(!statics) return 0;
 setvbuf(statics, 0, _IONBF, 0);
 double speed = 0.05, elapsed = 0.0;
 double delta_time = 1.0; // Remember dont divide by 0
 while(display_is_running())
 {
  double start_frame = get_time();

  display_poll_events();

  display_set_color(to_rgba(calculate_rgb(elapsed)));

  double fps = 1.0 / delta_time;
  char* string = to_string("%f", fps);
  fprintf(statics, "%s\n", string);
  term_vec2 size = {0};
  term_texture* texture = display_string_texture(string, strlen(string), &size, rgba_init(255,255,255,255), rgba_init(0,0,0,0));
  display_copy_texture(texture, pos_init(-1.0f, 1.0f), TEXTURE_MERGE_CROP);
  free(texture);
  free(string);

  display_show();
  elapsed += speed;

  delta_time = get_time() - start_frame;
 }
 display_free();
 fclose(statics);
 return 0;
}
