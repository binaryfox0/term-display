#include "swrz_scene.h"

#include <stdlib.h>
#include "swrz_alloc_priv.h"

swrz_error_t swrz__scene_init(
        swrz__scene_t *scene,
        const size_t mem_max)
{
    swrz_error_t err = SWRZ_ERR_OK;
    if(!scene || !mem_max)
        return SWRZ_ERR_PARAM;

    scene->mem_max = mem_max;
    swrz__arena_reset(&scene->arena);
    err = swrz__mutex_create(&scene->mutex);
    if(err != SWRZ_ERR_OK)
        return err;
    return SWRZ_ERR_OK;
}

swrz_error_t swrz__scene_resize(
        swrz__scene_t *scene,
        const uint32_t tile_x,
        const uint32_t tile_y)
{
    size_t tile_count = 0;
    void *bins_tmp = NULL;
    if(!scene || tile_x == 0 || tile_y == 0)
        return SWRZ_ERR_PARAM;

    tile_count = (size_t)tile_x * tile_y;
    if(tile_count < scene->bin_count)
        return SWRZ_ERR_OK;

    bins_tmp = swrz__realloc(
            scene->bins, 
            tile_count * sizeof(*scene->bins));
    if(!bins_tmp)
        return SWRZ_ERR_NO_MEM;

    scene->bins = bins_tmp;
    scene->bin_count = tile_count;
    scene->next_bin = 0;
    
    memset(scene->bins, 0, tile_count * sizeof(*scene->bins));

    return SWRZ_ERR_OK;
}

swrz__command_t *swrz__scene_create_command(
        swrz__scene_t *scene,
        const uint32_t tile_x,
        const uint32_t tile_y)
{
    swrz__command_t *command = NULL;
    size_t bin_idx = 0;
    swrz__bin_t *bin = NULL;
    if(!scene)
        return NULL;
    
    bin_idx = (size_t)tile_y * scene->tiles_x + tile_x;;
    if(bin_idx >= scene->bin_count)
        return NULL;

    if(scene->mem_usage > 
            scene->mem_max - sizeof(*command))
        return NULL;

    command = swrz__arena_malloc(&scene->arena, 
            sizeof(*command));
    if(!command)
        return NULL;
    memset(command, 0, sizeof(*command));

    bin = &scene->bins[bin_idx];
    if(bin->tail)
        bin->tail->next = command;
    else
        bin->head = command;
    bin->tail = command;
    return command;
}

swrz__bin_t *swrz__scene_next_bin(
        swrz__scene_t *scene)
{
    swrz__bin_t *bin = NULL;
    size_t index = 0;

    if(!scene)
        return NULL;

    swrz__mutex_lock(scene->mutex);
    if (scene->next_bin < scene->bin_count) 
    {
        index = scene->next_bin++;
        bin = &scene->bins[index];
    }

    swrz__mutex_unlock(scene->mutex);
    return bin;
}

void swrz__scene_destroy(
        swrz__scene_t *scene)
{
    if(!scene)
        return;

    swrz__free(scene->bins);
    swrz__mutex_destroy(scene->mutex);
    swrz__arena_destroy(&scene->arena);
    *scene = (swrz__scene_t){0};
}
