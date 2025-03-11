#ifndef TERMINAL_MATH_H
#define TERMINAL_MATH_H

#include "term_def.h"

#ifdef BUILD_WITH_CGLM
 #include "cglm/struct.h"
 
 typedef ivec2s term_vec2;
 typedef vec2s term_pos;

#else
 typedef struct { int x, y; } term_vec2;
 typedef struct { float x, y; } term_pos;
 
 
#endif

static inline term_vec2 vec2_init(int x, int y) { return (term_vec2) { .x = x, .y = y }; }
static inline term_pos pos_init(float x, float y) { return (term_pos) {.x = x, .y = y }; }

static inline term_vec2 ndc_to_pos(term_pos pos, term_vec2 size)
{
 return vec2_init(
  (int)((pos.x + 1.0) * 0.5f * size.x),
  (int)((1.0 - pos.y) * 0.5f * size.y)
 );
}

#endif