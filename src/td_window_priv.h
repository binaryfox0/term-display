#ifndef TD_WINDOW_PRIV_H
#define TD_WINDOW_PRIV_H

#include <td_def.h>
#include <td_window.h>
#include <td_event.h>

#include "td_utils.h"

typedef struct tdp_event_context tdp_event_context_t;
typedef struct td_renderer td_renderer_t;
typedef struct td_window
{
    td_irect rect;
    td_ivec2 display_pos;
    td_ivec2 pix_size;
    td_i32 orientation;
    struct {
        td_i32 xstart;
        td_i32 ystart;
        td_i32 xend;
        td_i32 yend;
    } fb_bound;

    td_color_mode_t color_mode;
    td_window_flags_t flags;

    td_bool should_close;

    td_renderer_t *renderer;
    tdp_event_context_t *event;
} td_window_t;

TD_INLINE td_ivec2 tdp_calculate_logical(
        td_window_t *window,
        const td_ivec2 term_size)
{
    td_ivec2 new_size = {0};

    new_size.x = term_size.x / window->pix_size.x;
    new_size.y = term_size.y / window->pix_size.y;

    return new_size;
}


TD_INLINE td_ivec2 tdp_apply_rotation(
        const td_window_t *window,
        const td_ivec2 size)
{
    td_ivec2 out = size;

    if(window->orientation % 2 == 1)
        TDP_SWAP(out.x, out.y, td_i32);

    return out;
}

void tdp_window_update_bound(td_window_t *window);

#endif
