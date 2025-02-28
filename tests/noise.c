#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "term_display.h"
#include "term_font.h"

#include "test_utils.h"

#ifdef TERMINAL_WINDOWS
 #include <bcrypt.h>
#endif

const u8 desired_channel = 4;
term_texture* generate_noise(term_vec2 size)
{
 term_texture* out = texture_create(0, desired_channel, size, 0, 0);
 u8* raw = texture_get_location(vec2_init(0,0), out);
 u64 byte = size.x * size.y * desired_channel;
#ifdef TERMINAL_WINDOWS
 if(BCryptGenRandom(0, raw, byte, BCRYPT_USE_SYSTEM_PREFERED_RNG))
 {
  texture_free(out);
  return 0;
 }
#else
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
#endif
 return out;
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
 double delta_time = 1.0; // Remember dont divide by 0
 u64 max_frame_count = 1;
 term_vec2 size = {0}; // Temporary
 while(display_is_running())
 {
  double start_frame = get_time();

  display_poll_events();

  display_option(display_size, 1, &size);
  term_texture* noise = 0;
  if((noise = generate_noise(size)))
  {
   display_copy_texture(noise, pos_init(-1.0f, 1.0f), TEXTURE_MERGE_CROP);
   texture_free(noise);
  }

  double fps = 1.0 / delta_time;
  char* string = to_string("%f", fps);
  fprintf(statics, "%s\n", string);
  term_texture* texture = display_string_texture(string, strlen(string), &size, rgba_init(0,0,0,255), rgba_init(255,255,255,255));
  display_copy_texture(texture, pos_init(-1.0f, 1.0f), TEXTURE_MERGE_CROP);
  texture_free(texture);
  free(string);

  display_show();

  delta_time = get_time() - start_frame;
 }
 display_free();
 fclose(statics);
 return 0;
}
