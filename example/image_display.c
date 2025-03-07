#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "term_display.h"
#include "term_font.h"

#include "example_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

char* get_program_name(char* in)
{
#ifdef TERMINAL_UNIX
 #include <libgen.h>
 return basename(in);
#elif TERMINAL_WINDOWS
 static char out[_MAX_FNAME] = {0};
 _splitpath_s(in, 0, 0, 0, 0, out, _MAX_FNAME, 0, 0);
 _splitpath_s(in, 0, 0, 0, 0, 0, 0, &out[strlen(out)], _MAX_EXT);
 return out;
#else
 return in;
#endif
}

int width, height;
term_texture* original_image = 0, *displayed_image = 0;
void resize_callback(term_vec2 new_size)
{
 height = new_size.y;
 if(displayed_image) free(displayed_image);
 displayed_image = texture_copy(original_image);
 texture_resize(displayed_image, vec2_init(width, height));
}

int main(int argc, char** argv)
{
 char* program_name = get_program_name(argv[0]);
 if(argc < 2)
 {
  printf("Usage: %s: <image>\n", program_name);
  return 1;
 }
 if(argc > 2)
 {
  printf("Error: %s:  Currently not support more than one image.\n", program_name);
  return 1;
 }
 int channel;
 u8* tmp = stbi_load(argv[1], &width, &height, &channel, 0);
 if(!tmp)
 {
  printf("Error: %s: Unable to load image file.\n", program_name);
  return 1;
 }
 original_image = texture_create(tmp, channel, vec2_init(width, height), 1, 0);
 if(!original_image)
 {
  printf("Error: %s: Unable to load image file.\n", program_name);
  free(tmp);
  return 1;
 }
 width = 0;

 if(display_init())
 {
  texture_free(original_image);
  texture_free(displayed_image);
  return 1;
 }

 u8 enable = 1;
 display_option(auto_resize, 0, &enable);

 term_vec2 current_size;
 display_option(display_size, 1, &current_size);
 resize_callback(current_size);
 display_set_resize_callback(resize_callback);

 while(display_is_running())
 {
  display_poll_events();
  display_set_color(rgba_init(0,0,0,255));
  display_copy_texture(displayed_image, pos_init(-1.f, 1.f), TEXTURE_MERGE_CROP);
  display_show();
 }

 texture_free(original_image);
 texture_free(displayed_image);
 display_free();

 return 0;
}
