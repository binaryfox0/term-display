#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "term_display.h"
#include "term_font.h"
#include <unistd.h>

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

term_texture* generate_noise(struct term_vec2 size)
{
 term_texture* out = texture_create(0, 3, size, 0, 0);
 u8* raw = texture_get_location(vec2_init(0,0), out);
 u64 byte = size.x * size.y * 3;
 int fd = 0;
 if((fd = open("/dev/urandom", O_RDONLY)) < 0)
 {
  texture_free(out);
  return 0;
 }
 if(read(fd, raw, byte) != byte)
 {
  close(fd);
  texture_free(out);
  return 0;
 }
 close(fd);
 return out;
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
 u64 max_frame_count = 1;
 double max_frame_time = 1.0 / 60; // 60 FPS
 struct term_vec2 size = {0}; // Temporary
 for(unsigned long long i = 0; i < max_frame_count; i++)
 {
  max_frame_count++;
  double start_frame = get_time();
  /*
  display_set_color(
  (struct term_color){
   .red = (u8)((sin(elapsed)+1)*127.5),
   .green = (u8)((sin(elapsed+(2*M_PI/3))+1)*127.5),
   .blue = (u8)((sin(elapsed+(4*M_PI/3))+1)*127.5)
  });
  */
  display_option(display_size, 1, &size);
  term_texture* noise = 0;
  if((noise = generate_noise(size)))
  {
   display_copy_texture(noise, pos_init(-1.0f, 1.0f), TEXTURE_MERGE_CROP);
   texture_free(noise);
  }

  double fps = 1.0 / delta_time;
  char* string = to_string(fps);
  fprintf(statics, "%s\n", string);
  term_texture* texture = display_string_texture(string, strlen(string), &size, rgba_init(0,0,0,255), rgba_init(255,255,255,255));
  display_copy_texture(texture, (struct term_pos){.x=-1.0f,.y=1.0f,}, TEXTURE_MERGE_CROP);
  texture_free(texture);
  free(string);

  display_show();
  elapsed += speed;

  /* FPS limiter start */
  //while((delta_time = get_time() - start_frame) < max_frame_time) {}
  delta_time = get_time() - start_frame;
 }
 display_free(0);
 fclose(statics);
 return 0;
}
