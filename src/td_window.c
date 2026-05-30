#include <td_window.h>

#include <stdio.h>

#include "td_priv.h"
#include "td_context.h"

#include "td_renderer_priv.h"

td_ivec2 tdp_calculate_logical(const td_ivec2 term_size)
{
    tdp_window_context_t *window = &tdp_ctx->window;
    td_ivec2 new_size = {0};

    new_size.x = term_size.x / window->pix_width;
    new_size.y = term_size.y / window->pix_height;

    if(window->rotation % 2 == 0) 
    { 
        window->fb_xend = new_size.x;
        window->fb_yend = new_size.y;
    } else {
        TDP_SWAP(new_size.x, new_size.y, td_i32);

        window->fb_xend = new_size.y;
        window->fb_yend = new_size.x;
    }

    return new_size;
}
void tdp_window_init(void)
{
    tdp_window_context_t *window = &tdp_ctx->window;
    window->pix_width = 2;
    window->pix_height = 1;
    window->pos = (td_ivec2){.x = 0, .y = 0};
    
    _pwrite(
        STDOUT_FILENO, 
        "\x1b[?25l"    // hide cursor
        "\x1b[?1049h"       // enable alternative buffer
        "\x1b[?1000h"       // enable mouse reporting
        "\x1b[?1003h"
        "\x1b[?1006h",      // enable SGR mode
        39
    );
}

void tdp_window_exit(void)
{
    fflush(stdout);
    _pwrite(STDOUT_FILENO, 
            "\x1b[?25h"
            "\x1b[0m"
            "\x1b[?1049l"
            "\x1b[?1000l"
            "\x1b[?1003l"
            "\x1b[?1006l", 
            43
    );
}

void tdp_window_resize(const td_ivec2 term_size)
{
    tdp_window_context_t *window = &tdp_ctx->window;
    td_ivec2 logical_size = {0};

    logical_size = tdp_calculate_logical(term_size);
    if(window->auto_resize != 0)
        tdp_renderer_resize(logical_size);
}

td_error_t td_window_clear(void)
{
    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;
    fflush(stdout);
    _pwrite(STDOUT_FILENO, 
        "\x1b[0m"              // Reset colors mode
        "\x1b[3J"                   // Clear saved line (scrollbuffer)
        "\x1b[H"                    // To position 0,0
        "\x1b[2J",                  // Clear entire screen
        16
    );
    return TD_ERR_OK;
}

td_error_t td_window_poll(void)
{
    tdp_window_context_t *window = &tdp_ctx->window;
    td_ivec2 current_size = {0};

    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;

    current_size = tdp_get_termsz();
    if(
            current_size.x != window->prev_size.x ||
            current_size.y != window->prev_size.y)
    {
        tdp_window_resize(current_size);
        window->prev_size = current_size;
    }
    return TD_ERR_OK;
}

TD_INLINE void tdp_display_cell(const td_u8 *c)
{
    switch(tdp_ctx->window.color_mode)
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
    }
}


td_error_t td_window_present(void)
{
    static td_u8 prev[4] = {0};

    tdp_window_context_t *window = 0;
    tdp_renderer_context_t *renderer = 0;
    td_i32 rot = 0;
    td_i32 px_w = 0;
    td_i32 px_h = 0;
    td_i32 fb_w = 0;
    td_i32 fb_h = 0;
    td_i32 ch   = 0;
    td_i32 xend = 0;
    td_i32 yend = 0;
    td_i32 x_stride = 0;
    td_i32 y_stride = 0;
    td_i32 term_x_base = 0;
    td_i32 term_y_base = 0;

    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;

    window = &tdp_ctx->window;
    renderer = &tdp_ctx->renderer;
    rot = window->rotation;
    px_w = window->pix_width;
    px_h = window->pix_height;
    fb_w = renderer->fb->size.x;
    fb_h = renderer->fb->size.y;
    ch   = (td_i32)renderer->fb->type;
    xend = window->fb_xend;
    yend = window->fb_yend;
    x_stride = ch;
    y_stride = fb_w * ch;
    term_x_base = window->pos.x * px_w + 1;
    term_y_base = window->pos.y * px_h + 1;

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

            for (int x = 0; x < xend; x++) {
                if (memcmp(prev, row_ptr, (size_t)ch) != 0) 
                {
                    tdp_display_cell(row_ptr);
                    memcpy(prev, row_ptr, (size_t)ch);
                }

                printf("%*s", px_w, "");
                row_ptr += dx;
            }

            printf("\x1b[1E");
        }
    }
    return TD_ERR_OK;
}
