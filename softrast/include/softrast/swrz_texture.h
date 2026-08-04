#ifndef SWRZ_TEXTURE_H
#define SWRZ_TEXTURE_H

#include <stdint.h>
#include <softrast/swrz_error.h>
#include <softrast/swrz_types.h>

typedef struct swrz_texture swrz_texture_t;

typedef enum
{
    SWRZ_PIXEL_RGBA,
    SWRZ__PIXEL_COUNT
} swrz_pixel_format_t;

typedef enum
{
    SWRZ_TEXTURE_RGBA8,
    SWRZ__TEXTURE_COUNT
} swrz_texture_format_t;

swrz_error_t swrz_texture_create(
        swrz_texture_t **tex);

uint32_t swrz_texture_get_width(
        swrz_texture_t *tex);

uint32_t swrz_texture_get_height(
        swrz_texture_t *tex);

void *swrz_texture_get_data(
        swrz_texture_t *tex);

uint32_t swrz_texture_get_row_pitch(
        swrz_texture_t *tex);

swrz_error_t swrz_texture_image_2d(
        swrz_texture_t *tex,
        const uint32_t width,
        const uint32_t height,
        const swrz_texture_format_t tex_fmt,
        const swrz_pixel_format_t pix_fmt,
        const swrz_data_type_t type,
        const void *data,
        const uint32_t row_pitch);

void swrz_texture_destroy(
        swrz_texture_t *tex);

#endif
