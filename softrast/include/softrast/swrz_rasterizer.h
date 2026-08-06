#ifndef SWRZ_RASTERIZER_H
#define SWRZ_RASTERIZER_H

#include <stdint.h>
#include <softrast/swrz_error.h>
#include <softrast/swrz_vertex.h>

typedef struct swrz_rasterizer swrz_rasterizer_t;

typedef enum
{
    SWRZ_PRIMITIVE_TRIANGLE,
    SWRZ__PRIMITIVE_COUNT
} swrz_primitive_t;

swrz_error_t swrz_rasterizer_create(
        swrz_rasterizer_t **rz,
        const uint32_t width,
        const uint32_t height);

struct swrz_texture *swrz_rasterizer_get_framebuffer(
        swrz_rasterizer_t *rz);

swrz_error_t swrz_rasterizer_bind_vao(
        swrz_rasterizer_t *rz,
        swrz_vertex_array_t *vao);

swrz_error_t swrz_rasterizer_bind_vertex_shader(
        swrz_rasterizer_t *rz,
        const swrz_vertex_shader_t shader);

swrz_error_t swrz_rasterizer_clear_color(
        swrz_rasterizer_t *rz,
        const float r,
        const float g,
        const float b,
        const float a);

swrz_error_t swrz_rasterizer_draw_array(
        swrz_rasterizer_t *rz,
        swrz_primitive_t primitive,
        const uint32_t first,
        const uint32_t count);

void swrz_rasterizer_destroy(
        swrz_rasterizer_t *rz);

#endif
