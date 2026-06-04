#ifndef TD_WINDOW_PRIV_H
#define TD_WINDOW_PRIV_H

#include <td_def.h>
#include <td_window.h>
#include <td_event.h>
    
#include "td_event_priv.h"

typedef struct td_renderer td_renderer_t;
typedef struct td_window
{
    td_irect rect;
    td_ivec2 pix_size;
    td_i32 rotation;
    td_ivec2 fb_bound;

    td_color_mode_t color_mode;
    td_window_flags_t flags;


    td_bool should_close;

    td_renderer_t *renderer;
    tdp_event_t event;
} td_window_t;

void tdp_window_update_bound(td_window_t *window);

#endif
