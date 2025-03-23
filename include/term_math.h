#ifndef TERMINAL_MATH_H
#define TERMINAL_MATH_H

#include "term_def.h"

#ifdef BUILD_WITH_CGLM
 #define CGLM_USE_ANONYMOUS_STRUCT 1
 #include "cglm/struct.h"
 
 typedef ivec2s term_ivec2;
 typedef vec2s term_vec2;
 typedef vec3s term_vec3;
 typedef vec4 term_vec4;

 typedef mat4 term_mat4;

 #define mat4_zero GLM_MAT4_ZERO
 #define perspective_mat4 glm_perspective
 #define mat4_mulv3 glm_mat4_mulv3

#else
 #include <math.h>
 #include <string.h>

 typedef struct { int x, y; } term_ivec2;
 typedef struct { float x, y; } term_vec2;
 typedef union { struct { float x, y, z; }; float raw[3];} term_vec3;

 typedef f32 term_vec4[4];
 typedef term_vec4 term_mat4[4];

 #define mat4_zero ((term_mat4){{0.0f, 0.0f, 0.0f, 0.0f}, \
                            {0.0f, 0.0f, 0.0f, 0.0f},  \
                            {0.0f, 0.0f, 0.0f, 0.0f},  \
                            {0.0f, 0.0f, 0.0f, 0.0f}})

 static inline void perspective_mat4(f32 fovy, f32 aspect, f32 nearZ, f32 farZ, term_mat4 dest)
 {
  memcpy(dest, mat4_zero, 16*sizeof(f32));
  f32 f  = 1.0f / tanf(fovy * 0.5f), fn = 1.0f / (nearZ - farZ);
  dest[0][0] = f / aspect;
  dest[1][1] = f;
  dest[2][2] = (nearZ + farZ) * fn;
  dest[2][3] = -1.0f;
  dest[3][2] = 2.0f * nearZ * farZ * fn;
 }

 static inline void mat4_mulv3(term_mat4 m, term_vec3 v, f32 last, term_vec3 dest)
 {
  dest.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * last;
  dest.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * last;
  dest.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * last;
 }
#endif

static inline term_ivec2 ivec2_init(int x, int y) { return (term_ivec2) { .x = x, .y = y }; }
static inline term_vec2 vec2_init(float x, float y) { return (term_vec2) {.x = x, .y = y }; }

static inline u8 vec2_equal(term_ivec2 a, term_ivec2 b) { return a.x == b.x && a.y == b.y; }

static inline term_ivec2 ndc_to_pos(term_vec2 pos, term_ivec2 size)
{
 return ivec2_init(
  (int)((pos.x + 1.0) * 0.5f * size.x),
  (int)((1.0 - pos.y) * 0.5f * size.y)
 );
}

#endif