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

#include <td_def.h>
#include <td_input.h>
#include <td_texture.h>
#include <td_renderer.h>
#include <td_font.h>

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
