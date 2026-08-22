#include "swrz_scene.h"

swrz_error_t swrz__scene_init(
        swrz__scene_t *scene,
        const size_t mem_max)
{
    if(!scene || !mem_max)
        return SWRZ_ERR_PARAM;

    scene->mem_max = mem_max;
    return SWRZ_ERR_OK;
}

swrz_error_t swrz__scene_resize(
        swrz__scene_t *scene,
        const uint32_t tile_x,
        const uint32_t tile_y)
{
    if(!scene || !tile_x || !tile_y)
        return SWRZ_ERR_PARAM;
    return SWRZ_ERR_OK;
}

void swrz__scene_destroy(
        swrz__scene_t *scene)
{
    if(!scene)
        return;
    *scene = (swrz__scene_t){0};
}
