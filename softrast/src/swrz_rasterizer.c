#include "softrast/swrz_rasterizer.h"

#include "swrz_utils.h"
#include "swrz_alloc_priv.h"
#include "swrz_texture_priv.h"
#include <string.h>

struct swrz_rasterizer
{
    swrz_texture_t *fb;
};

swrz_error_t swrz_rasterizer_create(
        swrz_rasterizer_t **rz,
        const uint32_t width,
        const uint32_t height)
{
    swrz_rasterizer_t *tmp = NULL;
    if(!rz)
        return SWRZ_ERR_PARAM;

    tmp = swrz__malloc(sizeof(*tmp));
    if(!tmp)
        return SWRZ_ERR_NO_MEM;

    swrz_texture_create(&tmp->fb);
    swrz_texture_image_2d(
            tmp->fb,
            width, height,
            SWRZ_TEXTURE_RGBA8,
            SWRZ_PIXEL_RGBA,
            SWRZ_DATA_U8,
            NULL,
            0);

    *rz = tmp;
    return SWRZ_ERR_OK;
}

swrz_texture_t *swrz_rasterizer_get_framebuffer(
        swrz_rasterizer_t *rz) {
    return rz ? rz->fb : NULL;
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

void swrz_rasterizer_destroy(
        swrz_rasterizer_t *rast)
{
    if(!rast)
        return;
    swrz_texture_destroy(rast->fb);
    swrz__free(rast);
}
