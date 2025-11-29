#include "td_renderer.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "td_rasterizer.h"
#include "td_main.h"
#include "td_priv.h"
#include "td_texture.h"

static const td_texture* tdp_current_tex = 0;
static int tdp_vertex_index = 0;
static tdp_vertex tdp_vertex_buffer[12] = {0};
static td_ivec2 tdp_terminal_size = {0};
td_display tdp_display = {0, 0, {0, 0}, {0, 0}};
static td_rgba tdp_bg_color = {0};
static td_rgba tdp_clear_color = {0};

extern td_i32 tdp_options[__td_opt_numeric_end__];

TD_INLINE void tdp_query_background(void)
{
    static const char *request = "\x1b]11;?\x1b\\";
    _pwrite(STDOUT_FILENO, request, strlen(request));
    if (!tdp_stdin_ready(4))
        return;
    char buffer[32] = { 0 };
    if (_pread(STDIN_FILENO, buffer, 32) == -1)
        return;
    char r[5] = { 0 }, b[5] = { 0 }, g[5] = { 0 };
    if (sscanf(buffer, "\x1B]11;rgb:%4[^/]/%4[^/]/%4[^;]", r, g, b) != 3)
        return;
    tdp_bg_color = (td_rgba){
        .r = (td_u8)(strtol(r, 0, 16) / 257),
        .g = (td_u8)(strtol(g, 0, 16) / 257),
        .b = (td_u8)(strtol(b, 0, 16) / 257),
        .a = 255
    };
}

td_ivec2 tdp_calculate_display_size(const td_ivec2 term_size)
{
    td_ivec2 new_size = {0};
    if(tdp_options[td_opt_display_rotate] % 2 == 0) {
        new_size = (td_ivec2){
            .x = term_size.x / tdp_options[td_opt_pixel_width],
            .y = term_size.y / tdp_options[td_opt_pixel_height]
        };
        tdp_display.sprop.yend = new_size.y;
        tdp_display.sprop.xend = new_size.x;
    } else {
        new_size = (td_ivec2){
            .x = term_size.y / tdp_options[td_opt_pixel_height],
            .y = term_size.x / tdp_options[td_opt_pixel_width]
        };
        tdp_display.sprop.yend = new_size.x;
        tdp_display.sprop.xend = new_size.y;
    }
    //tdp_display.size = td_ivec2_subtract(tdp_display.size, tdp_display.pos);
    return new_size;
}

TD_INLINE void tdp_reset_depth_buffer(void) {
    tdp_fill_buffer(tdp_display.depth, &(td_f32){ FLT_MAX }, calculate_pos(
        (td_ivec2){.y=tdp_display.fb->size.y}, tdp_display.fb->size.x, sizeof(td_f32)), sizeof(td_f32));
}

void tdp_resize_depth_buffer(void)
{
    if (!tdp_options[td_opt_depth_buffer])
        return;

    if (!tdp_display.fb->size.x || !tdp_display.fb->size.y) {
        if (tdp_display.depth) {
            free(tdp_display.depth);
            tdp_display.depth = NULL;
        }
        return;
    }

    td_u64 buf_size = calculate_pos((td_ivec2){.y=tdp_display.fb->size.y}, tdp_display.fb->size.x, sizeof(td_f32));
    td_f32 *tmp = (td_f32 *) realloc(tdp_display.depth, buf_size);
    if (!tmp) {
        return;
    }

    tdp_display.depth = tmp;
    tdp_reset_depth_buffer();
}

void tdp_resize_handle(const td_ivec2 term_size)
{
    td_ivec2 new_size = tdp_calculate_display_size(term_size);
    if(td_texture_set_buffer(tdp_display.fb, 0, new_size, 0) == td_false)
        return;                 // Uhhhh, how to continue processing without the tdp_display
    td_texture_fill(tdp_display.fb, tdp_clear_color);
    tdr_clear_term();
    tdp_resize_depth_buffer();
}

int tdp_renderer_init(const td_ivec2 term_size)
{
    tdp_query_background();
    tdp_clear_color = tdp_bg_color; // No color yet
    if (!(tdp_display.fb = td_texture_create(0, 3, (td_ivec2){0}, 1, 0))) // Empty texture
        return 1;
    _pwrite(STDOUT_FILENO, "\x1b[?25l\x1b[?1049h", 15); // Hide cursor and enable buffer
    _pwrite(STDOUT_FILENO, "\x1b[?1000h\x1b[?1006h", 16);                              // Enable mouse reporting, SGR mode
    tdp_resize_handle(term_size);
    return 0;
}

void tdp_renderer_exit(void)
{
    // Show the cursor again
    // Reset color / graphics mode
    fflush(stdout);             // Flush remaining data
    _pwrite(STDOUT_FILENO, "\x1b[?25h\x1b[0m\x1b[?1049l", 19);
    _pwrite(STDOUT_FILENO, "\x1b[?1000l\x1b[?1006l", 16);
    td_texture_destroy(tdp_display.fb);
    if(tdp_display.depth)
        free(tdp_display.depth);
}

void tdr_clear_term(void) {
    fflush(stdout);
    _pwrite(STDOUT_FILENO, 
        "\x1b[0m"                       // Reset colors mode
        "\x1b[3J"                       // Clear saved line (scrollbuffer)
        "\x1b[H"                        // To position 0,0
        "\x1b[2J",                      // Clear entire screen
        16
    );
}

void tdr_set_clear_color(const td_rgba clear_color)
{
    tdp_clear_color = td_pixel_blend(tdp_bg_color, clear_color);
    td_texture_fill(tdp_display.fb, tdp_clear_color);
}

void tdr_clear_framebuffer(void)
{
    td_texture_fill(tdp_display.fb, tdp_clear_color);
    tdp_reset_depth_buffer();
}

void tdr_draw_rect(const td_ivec2 top_left, const td_ivec2 bottom_right, const td_rgba color)
{
    tdp_vertex tlv = (tdp_vertex){.pos = top_left, .color = color};
    tdp_vertex trv = (tdp_vertex){.pos = (td_ivec2){.x = bottom_right.x, .y = top_left.y}, .color = color};
    tdp_vertex blv = (tdp_vertex){.pos = (td_ivec2){.x = top_left.x, .y = bottom_right.y}, .color = color};
    tdp_vertex brv = (tdp_vertex){.pos = bottom_right, .color = color};
    tdp_rasterize_triangle(tdp_display.fb, 0, tlv, trv, blv, 0);
    tdp_rasterize_triangle(tdp_display.fb, 0, trv, brv, blv, 0);
}

void tdr_copy_texture(const td_texture* tex, const td_ivec2 placement_pos)
{
    td_texture_merge(tdp_display.fb, tex, placement_pos,td_false);
}

void tdr_bind_texture(const td_texture *tex)
{
    tdp_current_tex = tex;
}

void tdr_add_vertex(const td_f32 *vertex, const tdr_vertex_attrib* vertex_attribs, const int attribs_count, const td_bool finalize)
{
    if(!attribs_count)
        return;
    tdp_vertex* curr = tdp_vertex_buffer + tdp_vertex_index;
    for(int i = 0; i < attribs_count; i++)
    {
        switch(vertex_attribs[i])
        {
        case TDRVA_POSITION_4D:
            curr->pos = ndc_to_pos((td_vec2){.x=vertex[0] / vertex[3], .y=vertex[1] / vertex[3]}, tdp_display.fb->size);
            curr->depth = vertex[2];
            vertex += 4;
            break;

        case TDRVA_POSITION_3D:
            curr->pos = ndc_to_pos((td_vec2){.x=vertex[0], .y=vertex[1]}, tdp_display.fb->size);
            curr->depth = vertex[2];
            vertex += 3;
            break;
        
        case TDRVA_POSITION_2D:
            curr->pos = ndc_to_pos((td_vec2){.x=vertex[0], .y=vertex[1]}, tdp_display.fb->size);
            vertex += 2;
            break;

        case TDRVA_COLOR_RGBA:
            curr->color = (td_rgba){
                .r = (td_u8)(vertex[0] * 255),
                .g = (td_u8)(vertex[1] * 255),
                .b = (td_u8)(vertex[2] * 255),
                .a = (td_u8)(vertex[3] * 255)
            };
            vertex += 4;
            break;

        case TDRVA_COLOR_RGB:
            curr->color = (td_rgba){
                .r = (td_u8)(vertex[0] * 255),
                .g = (td_u8)(vertex[1] * 255),
                .b = (td_u8)(vertex[2] * 255),
                .a = 255
            };
            vertex += 3;
            break;

        
        case TDRVA_UV_COORDS:
            curr->uv = (td_vec2){.x=vertex[0], .y=vertex[1]};
            vertex += 2;
            break;
        }
    }
    if(finalize)
        tdp_vertex_index++;
    if(tdp_vertex_index == 3) {
        tdp_rasterize_triangle(tdp_display.fb, tdp_display.depth,
            tdp_vertex_buffer[0], tdp_vertex_buffer[1], tdp_vertex_buffer[2],
            tdp_current_tex
        );
        memset(tdp_vertex_buffer, 0, sizeof(tdp_vertex_buffer[0]) * (unsigned long)tdp_vertex_index);
        tdp_vertex_index = 0;
    }
}

TD_INLINE td_u8 tdp_rgb_to_216(const td_u8 *c) {
    return (td_u8)(16 + (c[0] / 51 * 36) + (c[1] / 51 * 6) + (c[2] / 51));
}

TD_INLINE void tdp_display_cell(td_u8 *c)
{
    const int ch = tdp_display.fb->channel;
    const int dtype = tdp_options[td_opt_display_type];

    if (ch == 1) {
        if (dtype == td_display_grayscale_256) {
            int v = c[0];
            printf("\x1b[48;2;%d;%d;%dm", v, v, v);
        } else if (dtype == td_display_grayscale_24) {
            printf("\x1b[48;5;%dm", 232 + ((c[0] * 24) >> 8));
        }
    } else if (ch == 3) {
        if (dtype == td_display_truecolor) {
            printf("\x1b[48;2;%d;%d;%dm", c[0], c[1], c[2]);
        } else if (dtype == td_display_truecolor_216) {
            printf("\x1b[48;5;%dm", tdp_rgb_to_216(c));
        }
    }
}

// ANSI escape sequence https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
void tdr_render(void)
{
    printf("\x1b[H");
    int rot = tdp_options[td_opt_display_rotate];
    static td_u8 prev[3] = {0};

    for (int y = 0; y < tdp_display.sprop.yend; y++) {
        for (int yt = 0; yt < tdp_options[td_opt_pixel_height]; yt++) {

            for (int x = 0; x < tdp_display.sprop.xend; x++) {
                int tx = x, ty = y;

                switch (rot) {
                    case 1: /* 90 deg clockwise */      tx = y; ty = tdp_display.fb->size.y - 1 - x; break;
                    case 2: /* 180 deg */               tx = tdp_display.fb->size.x - 1 - x; ty = tdp_display.fb->size.y - 1 - y; break;
                    case 3: /* 270 deg clockwise */     tx = tdp_display.fb->size.x - 1 - y; ty = x; break;
                    default:                         break;
                }

                td_u8* ptr = tdp_display.fb->data + 
                    calculate_pos((td_ivec2){.x=tx, .y=ty}, tdp_display.fb->size.x, tdp_display.fb->channel);
                if (memcmp(prev, ptr, (td_u64)tdp_display.fb->channel)) {
                    tdp_display_cell(ptr);
                    memcpy(prev, ptr, (td_u64)tdp_display.fb->channel);
                }

                printf("%*s", tdp_options[td_opt_pixel_width], "");
            }

            printf("\x1b[1E");
        }
    }
}

