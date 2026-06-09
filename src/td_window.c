#include <td_window.h>
#include "td_window_priv.h"

#include <stdio.h>
#include <stdlib.h>

#include "td_context.h"
#include "td_utils.h"
#include "td_term.h"
#include "td_event_priv.h"
#include "td_texture_priv.h"
#include "td_renderer_priv.h"
#include "td_black_magic.h"


// Create window
td_window_t *td_window_create(
        const td_i32 x,
        const td_i32 y,
        const td_i32 w,
        const td_i32 h,
        const td_i32 orientation,
        const td_color_mode_t color_mode,
        const td_window_flags_t flags
)
{
    td_window_t *window = 0;
    td_ivec2 term_size = {0};

    // validate input
    if(
            x < 0 || y < 0 ||
            (flags & TD_WINDOW_FULLSCREEN ?
                (w < 0 || h < 0) :
                (w <= 0 || h <= 0)) ||
            TDP_OUT_RANGE(color_mode, 0, __TD_COLOR_MAX__ - 1))
        return 0;

    if(!tdp_ctx)
        return 0;

    if(tdp_ctx->window)
    {
        TDP_DEBUG_LOG("support for more than 1 window will be implmented later", );
        tdp_ctx->last_error = TD_ERR_GENERIC;
        return 0;
    }

    window = calloc(1, sizeof(*window));
    if(!window)
    {
        tdp_ctx->last_error = TD_ERR_OUT_OF_MEMORY;
        return 0;
    }

    window->pix_size.x = 2;
    window->pix_size.y = 1;

    window->orientation = orientation % 4;
    window->color_mode = color_mode;
    window->flags = flags;

    window->rect.x = x;
    window->rect.y = y;

    term_size = tdp_term_get_size();
    if(window->flags & TD_WINDOW_FULLSCREEN)
    {
        td_ivec2 logical_size = {0};

        logical_size = tdp_calculate_logical(window, term_size);
        TDP_DEBUG_LOG("term_size=%dx%d, logical_size=%dx%d",
                term_size.x, term_size.y,
                logical_size.x, logical_size.y);
        if(window->orientation & 1 != 0)
            TDP_SWAP(logical_size.x, logical_size.y, td_i32);
        TDP_DEBUG_LOG("window_size=%dx%d", 
                logical_size.x, logical_size.y);
        window->rect.w = logical_size.x;
        window->rect.h = logical_size.y;
    }
    else
    {
        window->rect.w = w;
        window->rect.h = h;
    }

    // default at the center of window
    window->event = tdp_event_create_context(
            (td_ivec2){
                .x = x + w / 2,
                .y = y + h / 2
            }
    );
    if(!window->event)
    {
        free(window);
        return 0;
    }

    // enter terminal UI mode
    tdp_tty_write(
            "\x1b[?1000h"
            "\x1b[?1003h"
            "\x1b[?1006h"
    );

    tdp_ctx->window = window;

    return window;
}

td_error_t td_window_resize(
        td_window_t *window,
        const td_ivec2 logical_size)
{
    td_error_t err = TD_ERR_OK;
    if(
            !window ||
            logical_size.x <= 0 ||
            logical_size.y <= 0
    )
        return TD_ERR_INVALID_ARG;

    err = tdp_renderer_resize(window->renderer, logical_size);
    if(err != TD_ERR_OK)
        return err;
    window->rect.w = logical_size.x;
    window->rect.h = logical_size.y;
    tdp_window_update_bound(window); 
    return TD_ERR_OK;
}

td_error_t td_window_set_size(
        td_window_t *window,
        const td_i32 width,
        const td_i32 height)
{
    td_error_t err = TD_ERR_OK;
    if(!window || width <= 0 || height <= 0)
        return TD_ERR_INVALID_ARG;
    
    err = tdp_renderer_resize(window->renderer, (td_ivec2){.x=width, .y=height});
    if(err != TD_ERR_OK)
        return err;
    
    window->rect.w = width;
    window->rect.h = height;
    tdp_window_update_bound(window);
    tdp_term_clear();
    return TD_ERR_OK;
}

td_error_t td_window_set_position(
        td_window_t *window,
        const td_i32 x,
        const td_i32 y)
{
    if(!window)
        return TD_ERR_INVALID_ARG;

    window->rect.x = x;
    window->rect.y = y;
    tdp_window_update_bound(window);
    tdp_term_clear();
    return TD_ERR_OK;
}

td_error_t td_window_set_orientation(
        td_window_t *window,
        const td_i32 orientation)
{
    td_error_t err = TD_ERR_OK;
    td_ivec2 new_size = {0};
    if(!window)
        return TD_ERR_INVALID_ARG;
    if((window->orientation % 2) == (orientation % 2))
        return TD_ERR_OK;
    // swap
    new_size.x = window->rect.h;
    new_size.y = window->rect.w;
    err = tdp_renderer_resize(window->renderer, new_size);
    if(err != TD_ERR_OK) 
        return err;
    window->rect.w = new_size.x;
    window->rect.h = new_size.y;
    window->orientation = orientation % 4;
    tdp_window_update_bound(window);
    tdp_term_clear();
    return TD_ERR_OK;
}

td_error_t td_window_set_resizable(
        td_window_t *window,
        const td_bool resizable)
{
    if(!window)
        return TD_ERR_INVALID_ARG;
    if(resizable)
        window->flags |= TD_WINDOW_RESIZABLE;
    else
        window->flags &= (td_u32)~TD_WINDOW_RESIZABLE;
    return TD_ERR_OK;
}

td_error_t td_window_set_should_close(
        td_window_t *window,
        const td_bool should_close)
{
    if(!window)
        return TD_ERR_INVALID_ARG;
    window->should_close = should_close;
    return TD_ERR_OK;
}

td_window_flags_t td_window_get_flags(td_window_t *window)
{
    return window ? window->flags : 0;
}

td_error_t td_window_get_position(
        td_window_t *window,
        td_i32 *x,
        td_i32 *y)
{
    if(!window || !x || !y)
        return TD_ERR_INVALID_ARG;

    *x = window->rect.x;
    *y = window->rect.y;

    return TD_ERR_OK;
}

td_error_t td_window_get_size(
        td_window_t *window,
        td_i32 *width,
        td_i32 *height)
{
    if(!window || !width || !height)
        return TD_ERR_INVALID_ARG;

    *width = window->rect.w;
    *height = window->rect.h;

    return TD_ERR_OK;
}

td_i32 td_window_get_orientation(td_window_t *window)
{
    return window ? window->orientation : 0;
}


td_bool td_window_should_close(
        td_window_t *window)
{
    return window ? window->should_close : TD_FALSE;
}

TD_INLINE void tdp_display_cell(
        const td_u8 *c,
        const td_color_mode_t cm)
{
    switch(cm)
    {
        case TD_COLOR_GRAYSCALE_24:
            printf("\x1b[48;5;%dm", 232 + ((c[0] * 24) >> 8));
            break;

        case TD_COLOR_GRAYSCALE_256:
            printf("\x1b[48;2;%d;%d;%dm", c[0], c[0], c[0]);
            break;

        case TD_COLOR_ANSI_216:
            printf("\x1b[48;5;%dm", 
                    16 +
                    ((((td_u16)c[0] * 161) >> 13) * 36) +
                    ((((td_u16)c[1] * 161) >> 13) * 6) +
                    (((td_u16)c[2] * 161) >> 13)
            );
            break;

        case TD_COLOR_TRUECOLOR:
            printf("\x1b[48;2;%d;%d;%dm", c[0], c[1], c[2]);
            break;

        default:
            break;
    }
}


td_error_t td_window_present(td_window_t *window)
{
    static td_u8 prev[4] = {0};

    td_renderer_t *renderer = 0;
    td_i32 rot = 0;
    td_i32 px_w = 0, px_h = 0;
    td_i32 fb_w = 0, fb_h = 0;
    td_i32 ch   = 0;
    td_i32 xstart = 0, ystart = 0;
    td_i32 xend = 0, yend = 0;
    td_i32 x_stride = 0, y_stride = 0;
    td_i32 term_x_base = 0, term_y_base = 0;

    
    if(!window)
        return TD_ERR_INVALID_ARG;

    renderer = window->renderer;
    rot = window->orientation;
    px_w = window->pix_size.x;
    px_h = window->pix_size.y;
    fb_w = renderer->fb->size.x;
    fb_h = renderer->fb->size.y;
    ch   = (td_i32)renderer->fb->type;
    xstart = window->fb_bound.xstart;
    ystart = window->fb_bound.ystart;
    xend = window->fb_bound.xend;
    yend = window->fb_bound.yend;
    x_stride = ch;
    y_stride = fb_w * ch;
    term_x_base = window->display_pos.x * px_w + 1;
    term_y_base = window->display_pos.y * px_h + 1;

    for (int y = ystart; y < yend; y++, term_y_base += px_h) 
    {
        for (int yt = 0; yt < px_h; yt++) 
        {
            td_u8 *row_ptr = 0;
            int dx = 0;

            printf("\x1b[%d;%dH",
                   term_y_base + yt,
                   term_x_base);

            switch (rot) 
            {
                /* 90deg CW */
                case 1:
                    row_ptr = renderer->fb->data
                            + (y * x_stride)
                            + ((fb_h - 1) * y_stride);
                    dx = -y_stride;
                    break;

                /* 180deg */
                case 2:
                    row_ptr = renderer->fb->data
                            + ((fb_w - 1) * x_stride)
                            + ((fb_h - 1 - y) * y_stride);
                    dx = -x_stride;
                    break;

                /* 270deg CW */
                case 3:
                    row_ptr = renderer->fb->data
                            + ((fb_w - 1 - y) * x_stride);
                    dx =  y_stride;
                    break;

                /* 0deg */
                default:
                    row_ptr = renderer->fb->data
                            + (y * y_stride);
                    dx =  x_stride;
                    break;
            }

            row_ptr += dx * xstart;
            for (int x = xstart; x < xend; x++) 
            {
                if (memcmp(prev, row_ptr, (size_t)ch) != 0) 
                {
                    tdp_display_cell(row_ptr, window->color_mode);
                    memcpy(prev, row_ptr, (size_t)ch);
                }

                printf("%*s", px_w, "");
                row_ptr += dx;
            }
        }
    }
    return TD_ERR_OK;
}

void td_window_destroy(td_window_t *window)
{
    if(!window)
        return;

    fflush(stdout);
    tdp_tty_write(
            "\x1b[?1000l"
            "\x1b[?1003l"
            "\x1b[?1006l"
    );
    tdp_event_destroy_context(window->event);
    free(window);
}

void tdp_window_update_bound(td_window_t *window)
{
    td_ivec2 term_size = {0};
    td_ivec2 logical_size = {0};
    td_ivec2 visible_size = {0};
 
    term_size = tdp_term_get_size();
    logical_size = tdp_calculate_logical(window, term_size);
     
    visible_size = tdp_apply_rotation(window, 
            (td_ivec2){.x = window->rect.w, .y = window->rect.h}
    );

    if(window->orientation == 0)
    {
        window->display_pos.x = TDP_MAX(window->rect.x, 0);
        window->display_pos.y = TDP_MAX(window->rect.y, 0);
        window->fb_bound.xstart = TDP_ABS(TDP_MIN(window->rect.x, 0));
        window->fb_bound.ystart = TDP_ABS(TDP_MIN(window->rect.y, 0));
        window->fb_bound.xend = TDP_MIN(visible_size.x, logical_size.x - window->display_pos.x);
        window->fb_bound.yend = TDP_MIN(visible_size.y, logical_size.y - window->display_pos.y);
    } else if(window->orientation == 1)
    {
        window->display_pos.x = TDP_MAX(logical_size.x - visible_size.x - window->rect.y, 0);
        window->display_pos.y = TDP_MAX(window->rect.x, 0);
        window->fb_bound.xstart = TDP_MAX(visible_size.x - (logical_size.x - TDP_ABS(window->rect.y)), 0);
        window->fb_bound.ystart = TDP_ABS(TDP_MIN(window->rect.x, 0));
        window->fb_bound.xend = visible_size.x + TDP_MIN(window->rect.y, 0);
        window->fb_bound.yend = TDP_MIN(visible_size.y, logical_size.y - window->display_pos.y);
    } else if(window->orientation == 2)
    {
        window->display_pos.x = TDP_MAX(logical_size.x - visible_size.x - window->rect.x, 0);
        window->display_pos.y = TDP_MAX(logical_size.y - visible_size.y - window->rect.y, 0);
        window->fb_bound.xstart = TDP_MAX(visible_size.x - (logical_size.x - TDP_ABS(window->rect.x)), 0);
        window->fb_bound.ystart = TDP_MAX(visible_size.y - (logical_size.y - TDP_ABS(window->rect.y)), 0);
        window->fb_bound.xend = TDP_MIN(visible_size.x, logical_size.x - window->display_pos.x);
        window->fb_bound.yend = TDP_MIN(visible_size.y, logical_size.y - window->display_pos.y);
    } else
    {
        window->display_pos.x = TDP_MAX(window->rect.y, 0);
        window->display_pos.y = TDP_MAX(logical_size.y - visible_size.y - window->rect.x, 0);
        //window->fb_bound.xstart = TDP_MAX(visible_size.x - (logical_size.x - TDP_ABS(window->rect.y)), 0);
        window->fb_bound.xstart = TDP_ABS(TDP_MIN(window->rect.y, 0));
        window->fb_bound.ystart = TDP_ABS(TDP_MIN(window->rect.x, 0));
        window->fb_bound.xend = TDP_MIN(logical_size.x - window->rect.y, visible_size.x);
        window->fb_bound.yend = TDP_MIN(visible_size.y, logical_size.y - window->display_pos.y);
    }

    TDP_DEBUG_LOG("rect.x=%d, rect.y=%d",
            window->rect.x, window->rect.y);
    TDP_DEBUG_LOG("rect.w=%d, rect.h=%d",
            window->rect.w, window->rect.h);
    TDP_DEBUG_LOG("display_pos.x=%d, display_pos.y=%d", 
            window->display_pos.x, window->display_pos.y);
    TDP_DEBUG_LOG("fb_bound.xstart=%d, fb_bound.xend=%d", 
            window->fb_bound.xstart, window->fb_bound.xend);
    TDP_DEBUG_LOG("fb_bound.ystart=%d, fb_bound.yend=%d",
            window->fb_bound.ystart, window->fb_bound.yend);
}
