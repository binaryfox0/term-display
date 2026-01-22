/*
MIT License

Copyright (c) 2025 binaryfox0 (Duy Pham Duc)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/** 
 * @file td_main.h
 * @brief Main header of term-display used to control the display and related operations.
 * 
 * This file provides the declarations for term-display settings, input handling,
 * and graphics-related operations. It includes functions for managing the display size,
 * key events, and rendering operations on a terminal-based display.
 */

#ifndef TD_DISPLAY_H
#define TD_DISPLAY_H

#include "td_def.h"
#include "td_texture.h"
#include "td_renderer.h"
 
 /**
  * @enum td_settings_t
  * @brief Enumeration for term-display settings.
  * 
  * These settings allow configuration of various aspects of the term-display, 
  * including size, depth buffer, and other display properties.
  */
typedef enum {
    td_opt_auto_resize = 0,             /**< Automatic resizing of the display */
    td_opt_pixel_width,                 /**< Pixel width of the display */
    td_opt_pixel_height,                /**< Pixel height of the display */
    td_opt_display_type,                /**< Type of display (grayscale, truecolor, etc.) */
    td_opt_display_rotate,              /**< Rotation of the display */
    td_opt_depth_buffer,                /**< Enable or disable depth buffer */
    td_opt_disable_stop_sig,            /**< Disable stop signal (SIGINT, SIGSTOP, etc) for the display */
    td_opt_shift_translate,             /**< Option to shift and translate the display */
    td_opt_enable_supersampling,        /**< Enable downscailing from bigger framebuffer */
    __td_opt_numeric_end__,             /**< End of numeric options */
    td_opt_display_size,                /**< Option for the display size */
    td_opt_display_pos,                 /**< The placement position of display (cells) */
    td_opt_supersampling_buffer_ratio   /**< The ratio between the supersampling buffer and the display */
} td_settings_t;

 /**
  * @enum display_types
  * @brief Enumeration for display types.
  * 
  * These values represent different display modes for rendering colors and graphics.
  */
typedef enum {
    td_display_grayscale_24,   /**< 24-color grayscale display */
    td_display_grayscale_256,  /**< 256-color grayscale display */
    td_display_truecolor_216,  /**< 216-color truecolor display */
    td_display_truecolor       /**< Full truecolor display */
} td_display_types;

/**
 * @enum td_key_state_t
 * @brief Enumeration for key states.
 * 
 * Represents the state of a key: whether it is released, pressed, or held.
 */
typedef enum { 
    td_key_release, /**< Key was released */
    td_key_press,   /**< Key was pressed */
    td_key_hold     /**< Key is being held down */
} td_key_state_t;

/**
 * @enum td_key_mod_t
 * @brief Enumeration for key modifier states.
 * 
 * These represent modifier keys that can be combined with other keys.
 */
typedef enum {
    td_mod_shift = 1, /**< Shift modifier key */
    td_mod_ctrl = 2,  /**< Control modifier key */
    td_mod_alt = 4    /**< Alt modifier key */
} td_key_mod_t;

/**
 * @enum td_key_token_t
 * @brief Enumeration for key tokens.
 * 
 * These values represent various key codes for keyboard keys.
 */
typedef enum {
    td_key_space = 32,        /**< Space ' ' key */
    td_key_astrophe = 39,     /**< Apostrophe '\'' key */
    td_key_comma = 44,        /**< Comma ',' key */
    td_key_minus,             /**< Minus '-' key */
    td_key_period,            /**< Period '.' key */
    td_key_slash,             /**< Slash '/' key */
    td_key_0,                 /**< '0' key */
    td_key_1,                 /**< '1' key */
    td_key_2,                 /**< '2' key */
    td_key_3,                 /**< '3' key */
    td_key_4,                 /**< '4' key */
    td_key_5,                 /**< '5' key */
    td_key_6,                 /**< '6' key */
    td_key_7,                 /**< '7' key */
    td_key_8,                 /**< '8' key */
    td_key_9,                 /**< '9' key */
    td_key_semicolon = 59,    /**< Semicolon key */
    td_key_equal = 61,        /**< Equal key */
    td_key_a = 65,            /**< 'A' key */
    td_key_b,                 /**< 'B' key */
    td_key_c,                 /**< 'C' key */
    td_key_d,                 /**< 'D' key */
    td_key_e,                 /**< 'E' key */
    td_key_f,                 /**< 'F' key */
    td_key_g,                 /**< 'G' key */
    td_key_h,                 /**< 'H' key */
    td_key_i,                 /**< 'I' key */
    td_key_j,                 /**< 'J' key */
    td_key_k,                 /**< 'K' key */
    td_key_l,                 /**< 'L' key */
    td_key_m,                 /**< 'M' key */
    td_key_n,                 /**< 'N' key */
    td_key_o,                 /**< 'O' key */
    td_key_p,                 /**< 'P' key */
    td_key_q,                 /**< 'Q' key */
    td_key_r,                 /**< 'R' key */
    td_key_s,                 /**< 'S' key */
    td_key_t,                 /**< 'T' key */
    td_key_u,                 /**< 'U' key */
    td_key_v,                 /**< 'V' key */
    td_key_w,                 /**< 'W' key */
    td_key_x,                 /**< 'X' key */
    td_key_y,                 /**< 'Y' key */
    td_key_z,                 /**< 'Z' key */
    td_key_left_bracket = 91, /**< Left bracket '[' */
    td_key_backslash,         /**< Backslash '\\' */
    td_key_right_bracket,     /**< Right bracket ']' */
    td_key_grave_accent = 96, /**< Grave accent '`' */
    td_key_escape = 256,      /**< Escape key */
    td_key_enter,             /**< Enter key */
    td_key_tab,               /**< Tab key */
    td_key_backspace,         /**< Backspace key */
    td_key_insert,            /**< Insert key */
    td_key_delete,            /**< Delete key */
    td_key_right,             /**< Right arrow key */
    td_key_left,              /**< Left arrow key */
    td_key_down,              /**< Down arrow key */
    td_key_up,                /**< Up arrow key */
    td_key_page_up,           /**< Page up key */
    td_key_page_down,         /**< Page down key */
    td_key_home,              /**< Home key */
    td_key_end,               /**< End key */
    td_key_f1 = 290,          /**< Function key F1 */
    td_key_f2,                /**< Function key F2 */
    td_key_f3,                /**< Function key F3 */
    td_key_f4,                /**< Function key F4 */
    td_key_f5,                /**< Function key F5 */
    td_key_f6,                /**< Function key F6 */
    td_key_f7,                /**< Function key F7 */
    td_key_f8,                /**< Function key F8 */
    td_key_f9,                /**< Function key F9 */
    td_key_f10,               /**< Function key F10 */
    td_key_f11,               /**< Function key F11 */
    td_key_f12                /**< Function key F12 */
} td_key_token_t;

/**
 * @typedef td_key_callback
 * @brief Typedef for key event callback function.
 * 
 * This typedef defines the function pointer type for handling key events.
 * The callback function is called when a key event occurs, passing the key code,
 * modifier keys, and key state (press, release, or hold).
 */
typedef void (*td_key_callback)(int key, int mods, td_key_state_t actions);

/**
 * @typedef td_mouse_callback
 * @brief Typedef for mouse event callback function.
 */
typedef void (*td_mouse_callback)(int xpos, int ypos, int key);

/**
 * @typedef td_resize_callback
 * @brief Typedef for resize event callback function.
 * 
 * This typedef defines the function pointer type for handling resize events.
 * The callback function is called when term-display is resized and the option <b>is enabled</b>.
 */
typedef void (*td_resize_callback)(td_ivec2 new_size);


/**
 * @brief Returns the copyright notice of the term-display library.
 * 
 * @return A string containing the copyright notice.
 */
const char *td_copyright_notice(void);

/**
 * @brief Initializes the term-display library.
 * 
 * @return A boolean indicating the success initialization.
 */
td_bool td_init(void);

/**
 * @brief Gets or sets the value of a term-display setting.
 * 
 * @param type The setting to query/modify.
 * @param get A boolean indicating whether to get or set the setting.
 * @param option A pointer to the setting value to retrieve or set.
 * 
 * @return A boolean indicating success or failure.
 */
td_bool td_option(td_settings_t type, td_bool get, void *option);

/**
 * @brief Checks if the term-display is running.
 * 
 * @return A boolean indicating display is shown to user.
 */
extern volatile td_bool __display_is_running;
TD_INLINE td_bool td_is_running(void) {
    return __display_is_running;
}

TD_INLINE void td_set_running_state(td_bool state) {
    __display_is_running = state;
}

/**
 * @brief Polls for events related to the display.
 */
void td_poll_events(void);

/**
 * @brief Sets the callback function to handle key events.
 * 
 * @param callback A function pointer to the key event handler.
 */
void td_set_key_callback(td_key_callback callback);

/**
 * @brief Sets the callback function to handle mouse events.
 * 
 * @param callback A function pointer to the mouse event handler.
 */
void td_set_mouse_callback(td_mouse_callback callback);

/**
 * @brief Sets the callback function to handle resize events.
 * 
 * @param callback A function pointer to the resize event handler.
 */
void td_set_resize_callback(td_resize_callback callback);

/**
 * @brief Frees any allocated resources used by the term-display library.
 */
void td_quit(void);

#endif /* TD_MAIN_H */
