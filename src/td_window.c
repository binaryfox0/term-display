#include <td_window.h>
#include "td_window_priv.h"

#include <stdio.h>
#include <stdlib.h>

#include "td_context.h"
#include "td_utils.h"
#include "td_term.h"
#include "td_texture_priv.h"
#include "td_renderer_priv.h"
#include "td_black_magic.h"
// Convert terminal size -> logical framebuffer size (NO rotation here anymore)
td_ivec2 tdp_calculate_logical(
        td_window_t *window,
        const td_ivec2 term_size)
{
    td_ivec2 new_size = {0};

    new_size.x = term_size.x / window->pix_size.x;
    new_size.y = term_size.y / window->pix_size.y;

    return new_size;
}


// Apply rotation once (single source of truth)
TD_INLINE td_ivec2 tdp_apply_rotation(
        const td_window_t *window,
        const td_ivec2 size)
{
    td_ivec2 out = size;

    if(window->rotation % 2 == 1)
        TDP_SWAP(out.x, out.y, td_i32);

    return out;
}


// Clamp size to renderer limits
TD_INLINE td_ivec2 tdp_clamp_to_renderer(
        const td_window_t *window,
        const td_ivec2 size)
{
    td_ivec2 out = size;

    out.x = tdp_min(out.x, window->renderer->size.x);
    out.y = tdp_min(out.y, window->renderer->size.y);

    return out;
}


// Restore terminal state
void tdp_window_exit(void)
{
    fflush(stdout);

    tdp_tty_write(
            "\x1b[?25h"
            "\x1b[0m"
            "\x1b[?1049l"
            "\x1b[?1000l"
            "\x1b[?1003l"
            "\x1b[?1006l"
    );
}


// Update framebuffer viewport
void tdp_window_update_layout(td_window_t *window, const td_ivec2 size)
{
    td_ivec2 clipped = size;

    clipped.x = tdp_min(clipped.x, window->renderer->size.x);
    clipped.y = tdp_min(clipped.y, window->renderer->size.y);

    window->fb_xend = clipped.x;
    window->fb_yend = clipped.y;
}


// Create window
td_window_t *td_window_create(
        const td_i32 x,
        const td_i32 y,
        const td_i32 w,
        const td_i32 h,
        const td_i32 rotation,
        const td_color_mode_t color_mode,
        const td_window_flags_t flags
)
{
    td_window_t *window = 0;

    td_ivec2 term_size = {0};
    td_ivec2 logical_size = {0};
    td_ivec2 rotated_size = {0};
    td_ivec2 final_size = {0};

    // validate input
    if(
            x < 0 || y < 0 ||
            (flags & TD_WINDOW_FULLSCREEN ?
                (w < 0 || h < 0) :
                (w <= 0 || h <= 0)) ||
            OUT_RANGE(color_mode, 0, __TD_COLOR_MAX__ - 1))
        return 0;

    window = calloc(1, sizeof(*window));
    if(!window)
        return 0;

    window->pix_size.x = 2;
    window->pix_size.y = 1;

    window->rotation = rotation;
    window->color_mode = color_mode;
    window->flags = flags;

    window->rect.x = x;
    window->rect.y = y;

    term_size = tdp_term_get_size();

    // pipeline
    logical_size = tdp_calculate_logical(window, term_size);
    rotated_size = tdp_apply_rotation(window, logical_size);

    // window rect
    if(window->flags & TD_WINDOW_FULLSCREEN)
    {
        window->rect.w = rotated_size.x;
        window->rect.h = rotated_size.y;
    }
    else
    {
        window->rect.w = w;
        window->rect.h = h;
    }

    // framebuffer bounds = min(logical space, window size)
    final_size.x = tdp_min(rotated_size.x, window->rect.w);
    final_size.y = tdp_min(rotated_size.y, window->rect.h);

    final_size = tdp_apply_rotation(window, final_size);

    window->fb_bound = final_size;

    // enter terminal UI mode
    tdp_tty_write(
            "\x1b[?25l"
            "\x1b[?1049h"
            "\x1b[?1000h"
            "\x1b[?1003h"
            "\x1b[?1006h"
    );

    return window;
}

td_error_t td_window_resize(const td_ivec2 logical_size)
{
    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;
    (void)logical_size;
    //tdp_renderer_resize(logical_size);
    //tdp_window_update_layout(); 
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
    td_i32 xend = 0, yend = 0;
    td_i32 x_stride = 0, y_stride = 0;
    td_i32 term_x_base = 0, term_y_base = 0;

    
    if(!window)
        return TD_ERR_INVALID_ARG;

    renderer = window->renderer;
    rot = window->rotation;
    px_w = window->pix_size.x;
    px_h = window->pix_size.y;
    fb_w = renderer->fb->size.x;
    fb_h = renderer->fb->size.y;
    ch   = (td_i32)renderer->fb->type;
    xend = window->fb_bound.x;
    yend = window->fb_bound.y;
    x_stride = ch;
    y_stride = fb_w * ch;
    term_x_base = window->rect.x * px_w + 1;
    term_y_base = window->rect.y * px_h + 1;

    for (int y = 0; y < yend; y++, term_y_base += px_h) 
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

            for (int x = 0; x < xend; x++) 
            {
                if (memcmp(prev, row_ptr, (size_t)ch) != 0) 
                {
                    tdp_display_cell(row_ptr, window->color_mode);
                    memcpy(prev, row_ptr, (size_t)ch);
                }

                printf("%*s", px_w, "");
                row_ptr += dx;
            }

            // XXX: this MUSTN'T be changed to \n, or results in bug
            printf("\x1b[1E");
        }
    }
    return TD_ERR_OK;
}

void td_window_destroy(td_window_t *window)
{
    if(!window)
        return;
    free(window);
}
