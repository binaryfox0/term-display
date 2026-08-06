#include "softrast/swrz_rasterizer.h"

#include <string.h>

#include "swrz_utils.h"
#include "swrz_alloc_priv.h"
#include "swrz_texture_priv.h"
#include "swrz_pool.h"

struct swrz_rasterizer
{
    swrz__pool_t pool;
    swrz_texture_t *fb;
    swrz_vertex_array_t *bound_vao;
    swrz_vertex_shader_t bound_vertex_shader;
};

swrz_error_t swrz_rasterizer_create(
        swrz_rasterizer_t **rz,
        const uint32_t width,
        const uint32_t height)
{
    swrz_error_t ret = SWRZ_ERR_OK;
    swrz_rasterizer_t *tmp = NULL;
    if(!rz)
        return SWRZ_ERR_PARAM;

    tmp = swrz__malloc(sizeof(*tmp));
    if(!tmp)
        return SWRZ_ERR_NO_MEM;

    SWRZ__CHECK(swrz_texture_create(&tmp->fb), ret, fail);
    SWRZ__CHECK(swrz_texture_image_2d(
            tmp->fb,
            width, height,
            SWRZ_TEXTURE_RGBA8,
            SWRZ_PIXEL_RGBA,
            SWRZ_DATA_U8,
            NULL,
            0), ret, fail);

    SWRZ__CHECK(swrz__pool_init(&tmp->pool), ret, fail);
    *rz = tmp;
    return SWRZ_ERR_OK;

fail:
    swrz_rasterizer_destroy(tmp);
    return ret;
}

swrz_texture_t *swrz_rasterizer_get_framebuffer(
        swrz_rasterizer_t *rz) {
    return rz ? rz->fb : NULL;
}

swrz_error_t swrz_rasterizer_bind_vao(
        swrz_rasterizer_t *rz,
        swrz_vertex_array_t *vao)
{
    if(!rz || !vao)
        return SWRZ_ERR_PARAM;
    rz->bound_vao = vao;
    return SWRZ_ERR_OK;
}

swrz_error_t swrz_rasterizer_bind_vertex_shader(
        swrz_rasterizer_t *rz,
        const swrz_vertex_shader_t shader)
{
    if(!rz || !shader)
        return SWRZ_ERR_PARAM;
    rz->bound_vertex_shader = shader;
    return SWRZ_ERR_OK;
}

swrz_error_t swrz_rasterizer_clear_color(
        swrz_rasterizer_t *rz,
        const float r,
        const float g,
        const float b,
        const float a)
{
    swrz_texture_t *fb = NULL;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t row_pitch = 0;
    uint8_t *data = NULL;
    uint32_t *row = NULL;

    uint32_t color = 0;
    if(!rz)
        return SWRZ_ERR_PARAM;

    fb = rz->fb;
    if(fb->format != SWRZ_TEXTURE_RGBA8)
        return SWRZ_ERR_UNHANDLED;

    width = fb->width;
    height = fb->height;
    row_pitch = fb->row_pitch;

    data = fb->data;
    row = fb->data;
    color = 
        ((uint32_t)(SWRZ__CLAMP(r, 0.0f, 1.0f) * 255.0f + 0.5f) << 24) |
        ((uint32_t)(SWRZ__CLAMP(g, 0.0f, 1.0f) * 255.0f + 0.5f) << 16) |
        ((uint32_t)(SWRZ__CLAMP(b, 0.0f, 1.0f) * 255.0f + 0.5f) << 8)  |
        ((uint32_t)(SWRZ__CLAMP(a, 0.0f, 1.0f) * 255.0f + 0.5f));

    for(uint32_t x = 0; x < width; x++)
        row[x] = color;
    for(uint32_t y = 1; y < height; y++)
        memcpy(data + y * row_pitch, data, row_pitch);

    return SWRZ_ERR_OK;
}

static void swrz__vertex_fetch(
        const swrz_vertex_array_t *vao,
        const uint32_t vertex_index,
        swrz_vertex_input_t *input)
{
    for(uint32_t i = 0; i < SWRZ_MAX_VERTEX_ATTRIBUTES; i++)
    {
        const swrz_vertex_attribute_t *attr = NULL;
        const swrz_vertex_binding_t *binding = NULL;
        const uint8_t *src = NULL;

        attr = &vao->attributes[i];
        if(!attr->enabled)
            continue;

        binding = &vao->bindings[attr->binding];
        if(
                attr->binding >= SWRZ_MAX_VERTEX_BINDINGS ||
                !SWRZ__ENUM_IN_RANGE(DATA, attr->type) ||
                !SWRZ__IN_RANGE(attr->components, 1, 4) ||
                attr->offset >= binding->stride)
            continue;

        src = (const uint8_t *)binding->data +
              vertex_index * binding->stride +
              attr->offset;

        switch(attr->type)
        {
        case SWRZ_DATA_F32:
        {
            float *out = input->attr->vec4;
            out[0] = 0.0f;
            out[1] = 0.0f;
            out[2] = 0.0f;
            out[3] = 1.0f;
            memcpy(out, src, attr->components * sizeof(float));
            break;
        }

        case SWRZ_DATA_U8:
        /*
        {
            const uint8_t *v = src;
            uint32_t c = 0;

            for(c = 0; c < attr->components; c++)
            {
                if(attr->normalized)
                    input->attr[i][c] = (float)v[c] / 255.0f;
                else
                    input->attr[i][c] = (float)v[c];
            }

            break;
        }
        */

        case SWRZ__DATA_COUNT:
        default:
            continue;
        }
    }
}
swrz_error_t swrz_rasterizer_draw_array(
        swrz_rasterizer_t *rz,
        swrz_primitive_t primitive,
        const uint32_t first,
        const uint32_t count)
{
    if(!rz || !SWRZ__ENUM_IN_RANGE(PRIMITIVE, primitive))
        return SWRZ_ERR_PARAM;
    if(!rz->bound_vao || !rz->bound_vertex_shader)
        return SWRZ_ERR_INVALID;

    for(uint32_t i = first; i < first + count; i++)
    {
        swrz_vertex_input_t input = {0};
        swrz_vertex_output_t output = {0};
        swrz__vertex_fetch(rz->bound_vao, i, &input);
        rz->bound_vertex_shader(&input, &output);
    }
    
    return SWRZ_ERR_OK;
}

void swrz_rasterizer_destroy(
        swrz_rasterizer_t *rast)
{
    if(!rast)
        return;
    swrz__pool_destroy(&rast->pool);
    swrz_texture_destroy(rast->fb);
    swrz__free(rast);
}
