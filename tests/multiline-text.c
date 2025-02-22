#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "term_display.h"
#include "term_font.h"

#include "test_utils.h"

char* to_string(double number)
{
 int len = snprintf(0, 0, "%f", number);
 char* string = (char*)malloc(len+1);
 string[len] = 0;
 snprintf(string, len+1, "%f", number);
 return string;
}

int main()
{
 u8 enable = 1;
 if(display_init())
  return 1;
 display_option(auto_resize, 0, &enable);
 FILE* statics = fopen("statics.txt", "w");
 setvbuf(statics, 0, _IONBF, 0);
 double delta_time = 1.0; // Remember dont divide by 0
 struct term_vec2 size = {0}; // Temporary
 const char* text = "\rHello\nWorld!";
 u64 frame_count = 0;
 while(display_is_running())
 {
  frame_count++;
  double start_frame = get_time();
 
  display_set_color(rgba_init(109, 154, 140, frame_count/7)); // Approximtely patina

  double fps = 1.0 / delta_time;
  char* string = to_string(fps);
  fprintf(statics, "%s\n", string);
  term_texture* texture = display_string_texture(string, strlen(string), &size, rgba_init(0,0,0,255), rgba_init(255,255,255,255));
  display_copy_texture(texture, (struct term_pos){.x=-1.0f,.y=1.0f,}, TEXTURE_MERGE_RESIZE);
  texture_free(texture);
  free(string);
  texture = display_string_texture(text, strlen(text), &size, rgba_init(255,255,255,255), rgba_init(0,0,0,127));
  display_copy_texture(texture, pos_init(-1.0f, 0.0f), TEXTURE_MERGE_CROP);
  texture_free(texture);
 /*
  texture = display_char_texture('d', rgba_init(255,255,255,255), rgba_init(0,0,0,255));
  display_copy_texture(texture, 4, vec2_init(3,5), pos_init(-1.0f, 0.0f));*/
  display_show();

  delta_time = get_time() - start_frame;
 }
 display_free(0);
 fclose(statics);
 return 0;
}
