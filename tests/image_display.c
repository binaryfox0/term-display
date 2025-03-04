#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "term_display.h"
#include "term_font.h"

#include "test_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

char* get_program_name(const char* in)
#ifdef TERMINAL_UNIX
 #include <libgen.h>
 return basename(in);
#elif TERMINAL_WINDOWS
 char out[_MAX_FNAME] = {0};
 _splitpath_s(in, 0, 0, 0, 0, out, _MAX_FNAME, 0, 0);
 _splitpath_s(in, 0, 0, 0, 0, 0, 0, &out[strlen(out)], _MAX_EXT);
 return out;
#else
 return in;
#endif
}

int main(int argc, char** argv)
{
 if(argc
 u8 enable = 1;
 if(display_init())
  return 1;
 display_option(auto_resize, 0, &enable);

 term_texture* image = stbi_load(

 while(display_is_running())
 {
 }
 display_free();
 return 0;
}
