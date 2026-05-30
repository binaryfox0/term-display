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
