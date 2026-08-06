#ifndef SWRZ_SHADER_H
#define SWRZ_SHADER_H

#include <stdbool.h>
#include <stdint.h>
#include <softrast/swrz_types.h>

#define SWRZ_MAX_VERTEX_BINDINGS 8
#define SWRZ_MAX_VERTEX_ATTRIBUTES 16

typedef float swrz_vec4[4];

typedef struct
{
    const void *data;
    uint32_t stride;
} swrz_vertex_binding_t;

typedef struct 
{
    bool enabled;
    uint32_t binding;
    uint32_t offset;
    swrz_data_type_t type;
    uint8_t components;
    bool normalized;
} swrz_vertex_attribute_t;

typedef struct swrz_vertex_array
{
    swrz_vertex_binding_t bindings[SWRZ_MAX_VERTEX_BINDINGS];
    swrz_vertex_attribute_t attributes[SWRZ_MAX_VERTEX_ATTRIBUTES];
} swrz_vertex_array_t;

typedef struct
{
    union
    {
        swrz_vec4 vec4;
    } attr[SWRZ_MAX_VERTEX_ATTRIBUTES];
} swrz_vertex_input_t;

typedef struct
{
    swrz_vec4 position;
} swrz_vertex_output_t;

typedef void (*swrz_vertex_shader_t)(
        const swrz_vertex_input_t *input,
        swrz_vertex_output_t *output);

#endif
