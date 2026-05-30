#include <td_option.h>

#include "td_context.h"

#define TDP_OPTION_SET(dst, src, type) ((dst) = *(type*)(src))
#define TDP_OPTION_GET(dst, src, type) (*(type*)dst = (src))
#define TDP_OPTION_SET_GET(dst, src, type, is_set) \
    { \
        if((is_set)) \
            TDP_OPTION_SET((dst), (src), type); \
        else \
            TDP_OPTION_GET((src), (dst), type); \
    }

#define TDP_OPTION_SET_GET_I32(dst, src, begin, end, is_set) \
    { \
        if((is_set)) \
        { \
            td_i32 tmp = *(td_i32*)(src); \
            if((begin) < (end) && OUT_RANGE(tmp, (begin), (end))) \
                goto invalid; \
            TDP_OPTION_SET((dst), (src), td_i32); \
        } \
        else \
            TDP_OPTION_GET((src), (dst), td_i32); \
    }

td_error_t td_option(
        const td_option_t opt, 
        const td_bool is_set, 
        void *ptr) 
{
    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;
    switch (opt) {
        case TD_OPT_AUTO_RESIZE:
            TDP_OPTION_SET_GET(tdp_ctx->window.auto_resize, 
                    ptr, td_bool, is_set);
            break;

        case TD_OPT_PIXEL_WIDTH:
            TDP_OPTION_SET_GET_I32(tdp_ctx->window.pix_width, 
                    ptr, 1, -1, is_set);
            break;

        case TD_OPT_PIXEL_HEIGHT:
            TDP_OPTION_SET_GET_I32(tdp_ctx->window.pix_height, 
                    ptr, 1, -1, is_set);
            break;

        case TD_OPT_WINDOW_SIZE: 
        {
            td_ivec2 tmp = {0};
            if(!is_set)
            {
                TDP_OPTION_GET(ptr, tdp_ctx->window.size, td_ivec2);
                break;
            }

            tmp = *(td_ivec2*)ptr;
            if(tmp.x <= 0 || tmp.y <= 0)
                goto invaid;

            
            break;
        }

        case td_opt_display_pos: {
            OPT_GET(td_ivec2, tdp_display.pos);
            OPT_SET(td_ivec2, tdp_display.pos);
            tdp_resize_handle(tdp_term_size);
            break;
        }

        case TD_OPT_COLOR_MODE: {
            OPT_GET(td_color_mode_t, tdp_options[type]);
            td_color_mode_t tmp = *(td_color_mode_t*)ptr;
            td_u8 new_channel = 3;
            switch(tmp) {
            case td_display_grayscale_24:
            case td_display_grayscale_256: new_channel = 1; break;
            case td_display_truecolor_216:
            case td_display_truecolor: new_channel = 3; break;
            default:
                return TD_FALSE;
            }
            tdp_options[type] = (td_i32)tmp;
            td_texture_convert(tdp_display.fb, new_channel);
            break;
        }

        case TD_OPT_DISPLAY_ROTATION: {
            OPT_GET(td_u8, tdp_options[type]);
            OPT_SET(td_u8, tdp_options[type]) % 4;
            tdp_resize_handle(tdp_term_size);
            break;
        }

        case TD_OPT_DEPTH_BUFFER: {
            OPT_GET(td_u8, tdp_options[type]);
            if((OPT_SET(td_u8, tdp_options[type]))){
                tdp_resize_depth_buffer();
            } else {
                if(tdp_display.depth) free(tdp_display.depth);
                tdp_display.depth = NULL;
            }
            break;
        }

        case TD_OPT_DISABLE_STOP_SIG:
        {
            td_bool tmp = *(td_bool*)ptr;
            if(tdp_enable_stop_sig(!tmp) == TD_FALSE)
                return 1;
            tdp_options[type] = (td_i32)tmp;
            break;
        }

        case TD_OPT_SHIFT_TRANSLATE: {
            OPT_GET(td_bool, tdp_shift_translate);
            OPT_SET(td_bool, tdp_shift_translate);
            break;
        }

        default:
            return TD_ERR_INVALID_ARG;
    }

    return TD_ERR_OK;

invalid;
    return TD_ERR_INVALID_ARG;
}
