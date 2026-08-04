#ifndef SWRZ_SHADER_H
#define SWRZ_SHADER_H

#include <stdbool.h>
#include <stdint.h>
#include <softrast/swrz_types.h>

#define SWRZ_MAX_VERTEX_BINDINGS 8
#define SWRZ_MAX_VERTEX_ATTRIBUTES 16

typedef struct swrz_vertex_array
{
    struct {
        const void *data;
        uint32_t stride;
    } bindings[SWRZ_MAX_VERTEX_BINDINGS];

    struct {
        bool enabled;
        uint32_t binding;
        uint32_t offset;
        swrz_data_type_t type;
        uint8_t components;
        bool normalized;
    } attributes[SWRZ_MAX_VERTEX_ATTRIBUTES];
} swrz_vertex_array_t;

typedef struct
{
    union
    {
        float f32[4];
    } attr[SWRZ_MAX_VERTEX_ATTRIBUTES];
} swrz_vertex_input_t;

typedef struct
{
    float position[4];
} swrz_vertex_output_t;

typedef void (*swrz_vertex_shader_t)(
        const swrz_vertex_input_t *input,
        swrz_vertex_output_t *output);

#endif
