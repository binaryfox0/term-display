#ifndef SWRZ_SCENE_H
#define SWRZ_SCENE_H

#include <stdint.h>
#include <softrast/swrz_error.h>
#include "swrz_arena.h"

typedef struct swrz__command
{
    struct swrz__command *next;
} swrz__command_t;

typedef struct
{
    swrz__command_t *head;
    swrz__command_t *tail;
} swrz__bin_t;

typedef struct swrz__scene
{
    swrz__arena_t arena;
    swrz__bin_t *bins;
    size_t count;
    size_t mem_usage;
    size_t mem_max;
} swrz__scene_t;

swrz_error_t swrz__scene_init(
        swrz__scene_t *scene,
        const size_t mem_max);

swrz_error_t swrz__scene_resize(
        swrz__scene_t *scene,
        const uint32_t tile_x,
        const uint32_t tile_y);

void swrz__scene_destroy(
        swrz__scene_t *scene);

#endif
