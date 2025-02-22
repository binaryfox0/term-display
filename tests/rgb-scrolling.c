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

char* to_string(double number)
{
 int len = snprintf(0, 0, "%f", number);
 char* string = (char*)malloc(len+1);
 memset(string, 0, len+1);
 snprintf(string, len+1, "%f", number);
 return string;
}

struct term_rgb calculate_rgb(double d)
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
 setvbuf(statics, 0, _IONBF, 0);
 double speed = 0.05, elapsed = 0.0;
 double delta_time = 1.0; // Remember dont divide by 0
 while(display_is_running())
 {
  double start_frame = get_time();
  display_set_color(to_rgba(calculate_rgb(elapsed)));
/*  display_copy_texture(
   (u8*)&texture_Digit[0], 4, (struct term_vec2){.x = 3, .y = 5}, (struct term_pos){.x = -1.0f, .y = 1.0f});*/
//  display_set_color((struct term_color){.red=100,.green=50,.blue=255});

  double fps = 1.0 / delta_time;
  char* string = to_string(fps);
  fprintf(statics, "%s\n", string);
  struct term_vec2 size = {0};
  term_texture* texture = display_string_texture(string, strlen(string), &size, rgba_init(255,255,255,255), rgba_init(0,0,0,0));
  display_copy_texture(texture, (struct term_pos){.x=-1.0f,.y=1.0f,}, TEXTURE_MERGE_CROP);
  free(texture);
  free(string);

  display_show();
  elapsed += speed;

  delta_time = get_time() - start_frame;
 }
 display_free(0);
 fclose(statics);
 return 0;
}
