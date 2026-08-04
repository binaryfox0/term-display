#include "softrast/swrz_texture.h"
#include "swrz_texture_priv.h"

#include "swrz_utils.h"
#include "swrz_alloc_priv.h"
#include <string.h>

swrz_error_t swrz_texture_create(
        swrz_texture_t **tex)
{
    swrz_texture_t *tmp = NULL;
    if(!tex)
        return SWRZ_ERR_PARAM;
    tmp = swrz__calloc(1, sizeof(*tmp));
    if(!tmp)
        return SWRZ_ERR_NO_MEM;
    *tex = tmp;
    return SWRZ_ERR_OK;
}

uint32_t swrz_texture_get_width(
        swrz_texture_t *tex) {
    return tex ? tex->width : 0;
}

uint32_t swrz_texture_get_height(
        swrz_texture_t *tex) {
    return tex ? tex->height : 0;
}

void *swrz_texture_get_data(
        swrz_texture_t *tex) {
    return tex ? tex->data : NULL;
}

uint32_t swrz_texture_get_row_pitch(
        swrz_texture_t *tex) {
    return tex ? tex->row_pitch : 0;
}

swrz_error_t swrz_texture_image_2d(
        swrz_texture_t *tex,
        const uint32_t width,
        const uint32_t height,
        const swrz_texture_format_t tex_fmt,
        const swrz_pixel_format_t pix_fmt,
        const swrz_data_type_t type,
        const void *data,
        const uint32_t row_pitch)
{
    static const uint32_t format_to_comp[SWRZ__PIXEL_COUNT] =
    {
        [SWRZ_PIXEL_RGBA] = 4
    };

    static const uint32_t type_to_bits[SWRZ__DATA_COUNT] =
    {
        [SWRZ_DATA_U8] = 8,
    };

    uint32_t comp = 0;
    uint32_t bits = 0;
    uint32_t bytes_per_pixel = 0;

    uint32_t src_pitch = 0;
    uint32_t dst_pitch = 0;
    size_t size = 0;

    if(
            !tex ||
            !SWRZ__ENUM_IN_RANGE(TEXTURE, tex_fmt) ||
            !SWRZ__ENUM_IN_RANGE(PIXEL, pix_fmt) ||
            !SWRZ__ENUM_IN_RANGE(DATA, type) ||
            width == 0 ||
            height == 0)
        return SWRZ_ERR_PARAM;

    comp = format_to_comp[pix_fmt];
    bits = type_to_bits[type];
    if(comp == 0 || bits == 0)
        return SWRZ_ERR_UNHANDLED;

    if(tex_fmt != SWRZ_TEXTURE_RGBA8)
        return SWRZ_ERR_UNHANDLED;

    if(tex->data)
        swrz__free(tex->data);

    bytes_per_pixel = (comp * bits) / 8;
    dst_pitch = width * bytes_per_pixel;
    size = (size_t)dst_pitch * height;

    tex->data = swrz__calloc(
            size,
            sizeof(uint8_t));

    if(!tex->data)
        return SWRZ_ERR_NO_MEM;
        
    tex->width = width;
    tex->height = height;
    tex->format = tex_fmt;
    tex->row_pitch = dst_pitch;

    src_pitch = row_pitch;
    if(src_pitch == 0)
        src_pitch = dst_pitch;

    if(data)
    {
        const uint8_t *src = data;
        uint8_t *dst = tex->data;

        for(uint32_t y = 0; y < height; y++)
        {
            memcpy(
                dst + (size_t)y * dst_pitch,
                src + (size_t)y * src_pitch,
                dst_pitch);
        }
    }

    return SWRZ_ERR_OK;
}

void swrz_texture_destroy(
        swrz_texture_t *tex)
{
    if(!tex)
        return;
    swrz__free(tex->data);
    swrz__free(tex);
}
