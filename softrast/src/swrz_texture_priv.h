#ifndef SWRZ_TEXTURE_PRIV_H
#define SWRZ_TEXTURE_PRIV_H

#include "softrast/swrz_texture.h"

struct swrz_texture
{
    uint32_t width;
    uint32_t height;
    swrz_texture_format_t format;
    void *data;
    uint32_t row_pitch;
};

#endif

