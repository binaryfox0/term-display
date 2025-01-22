#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "term_display.h"
#include "term_font.h"
#include <unistd.h>
#include <string.h>

#define M_PI 3.14159265358979323846

// In seconds
double get_time()
{
 struct timespec ts;
 clock_gettime(CLOCK_MONOTONIC, &ts);
 return ts.tv_sec + ((double)ts.tv_nsec / 1000000000);
}

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
 display_option(auto_resize, 0, &enable);
 FILE* statics = fopen("statics.txt", "w");
 setvbuf(statics, 0, _IONBF, 0);
 double speed = 0.05, elapsed = 0.0;
 double delta_time = 1.0; // Remember dont divide by 0
 u64 max_frame_count = 50;
 double max_frame_time = 1.0 / 60; // 60 FPS
 for(unsigned long long i = 0; i < max_frame_count; i++)
 {
  max_frame_count++;
  double start_frame = get_time();
  display_set_color(to_rgba(calculate_rgb(elapsed)));
/*  display_copy_texture(
   (u8*)&texture_Digit[0], 4, (struct term_vec2){.x = 3, .y = 5}, (struct term_pos){.x = -1.0f, .y = 1.0f});*/
//  display_set_color((struct term_color){.red=100,.green=50,.blue=255});

  double fps = 1.0 / delta_time;
  char* string = to_string(fps);
  fprintf(statics, "%s\n", string);
  struct term_vec2 size = {0};
  u8* texture = display_string_texture(string, strlen(string), &size, rgba_init(255,255,255,255), rgba_init(0,0,0,0));
  display_copy_texture(texture, 4, size, (struct term_pos){.x=-1.0f,.y=1.0f,});
  free(texture);
  free(string);

  display_show();
  elapsed += speed;

  /* FPS limiter start */
  while((delta_time = get_time() - start_frame) < max_frame_time) {}
 }
 display_free(0);
 fclose(statics);
 return 0;
}
