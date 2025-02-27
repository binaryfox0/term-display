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


FILE* statics = 0;
term_pos object_pos = (term_pos) { .x = 0.0f, .y = 0.0f };
const float speed = 0.1f;
void key_callback(int key, int mods, key_state state)
{
 if(key == term_key_page_up)
 {
  display_free();
  exit(0);
 }
 if(key == term_key_w || key == term_key_up)
  object_pos.y+= speed;
 if(key == term_key_a || key == term_key_left)
  object_pos.x-= speed;
 if(key == term_key_s || key == term_key_down)
  object_pos.y-= speed;
 if(key == term_key_d || key == term_key_right)
  object_pos.x+= speed;
}

int main()
{
 u8 enable = 1;
 if(display_init())
  return 1;
 display_option(auto_resize, 0, &enable);
 statics = fopen("statics.txt", "w");
 setvbuf(statics, 0, _IONBF, 0);
 double delta_time = 1.0; // Remember dont divide by 0
 term_vec2 size = {0}; // Temporary
 const char* text = "\rHello\nWorld!";
 u64 frame_count = 0;
 display_set_key_callback(key_callback);

 term_texture* object = texture_create(
  (u8[25])
  {
   255, 255, 255, 255, 255,
   255, 0,   255, 0,   255,
   255, 255, 255, 255, 255,
   255, 0,   0,   0,   255,
   255, 255, 255, 255, 255,
  }, 1, vec2_init(5,5), 0, 0
 );
 while(display_is_running())
 {
  frame_count++;
  double start_frame = get_time();

  display_set_color(rgba_init(109, 154, 140, frame_count/7)); // Approximtely patina

  display_poll_events();

  double fps = 1.0 / delta_time;
  char* string = to_string(fps);
  fprintf(statics, "%s\n", string);
  term_texture* texture = display_string_texture(string, strlen(string), &size, rgba_init(0,0,0,255), rgba_init(255,255,255,255));
  display_copy_texture(texture, pos_init(-1.0f, 1.0f), TEXTURE_MERGE_RESIZE);
  texture_free(texture);
  free(string);
  texture = display_string_texture(text, strlen(text), &size, rgba_init(255,255,255,255), rgba_init(0,0,0,127));
  display_copy_texture(texture, pos_init(-1.0f, 0.0f), TEXTURE_MERGE_CROP);
  texture_free(texture);
  display_copy_texture(object, object_pos, TEXTURE_MERGE_CROP);
 /*
  texture = display_char_texture('d', rgba_init(255,255,255,255), rgba_init(0,0,0,255));
  display_copy_texture(texture, 4, vec2_init(3,5), pos_init(-1.0f, 0.0f));*/
  display_show();

  delta_time = get_time() - start_frame;
 }
 texture_free(object);
 display_free();
 fclose(statics);
 return 0;
}
