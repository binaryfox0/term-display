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

static inline u8 is_landscape(term_vec2 size)
{
 return size.x > size.y;
}

term_vec2 ratio_new_size(const term_vec2 old, const term_vec2 size)
{
 if(!size.x) return vec2_init((old.x * size.y) / old.y ,size.y);
 if(!size.y) return vec2_init(size.x, (old.y * size.x) / old.x);
 return size;
}

u8 vec2_larger(term_vec2 vec1, term_vec2 vec2)
{
 return vec1.x > vec2.x || vec1.y > vec2.y;
}

void resize_callback(term_vec2 new_size)
{
 if(width) width = new_size.x;
 if(height) height = new_size.y;

 term_vec2 tmp = ratio_new_size(texture_get_size(original_image), vec2_init(width, height));
 if(vec2_larger(tmp, new_size))
 {
  if(width) { height = new_size.y; width = 0; }
  if(height) { width = new_size.x; height = 0; }
 }

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

 if(display_init() || start_logging("statics.txt"))
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

 double delta_time = 1.0, last_log = get_time();
 while(display_is_running())
 {
  double start_frame = get_time();
  double fps = (delta_time > 0) ? (1.0 / delta_time) : 0.0;

  display_poll_events();
  display_set_color(rgba_init(0,0,0,255));
  display_copy_texture(displayed_image, pos_init(-1.f, 1.f), TEXTURE_MERGE_CROP);
  display_show();

  delta_time = get_time() - start_frame;
  if(start_frame - last_log >= LOG_INTERVAL)
  {
   char* string = to_string("%f", fps);
   if(string)
   {
    write_log("FPS: %s", string);
    last_log = get_time();
    free(string);
   }
  }
 }

 texture_free(original_image);
 texture_free(displayed_image);
 display_free();
 stop_logging();

 return 0;
}
