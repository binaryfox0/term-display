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
#ifndef TD_EVENT_H
#define TD_EVENT_H

#include <td_def.h>
#include <td_error.h>

#define TD_DEFINE_CALLBACK_SETTER(type) \
    td_error_t td_set_##type##_callback(\
            td_window_t *window, \
            const td_##type##_callback_t callback)

typedef struct td_window td_window_t;

typedef void (*td_resize_callback_t)(td_ivec2 size);

/**
 * @enum td_key_action
 * @brief Enumeration for key actions.
 * 
 * Represents the state of a key: whether it is released, pressed, or held.
 */
typedef enum td_key_action
{ 
    TD_ACTION_RELEASE, /**< Key was released */
    TD_ACTION_PRESS,   /**< Key was pressed */
    TD_ACTION_HELD     /**< Key is being held down */
} td_key_action_t;

/**
 * @enum td_key_mod_t
 * @brief Enumeration for key modifier states.
 * 
 * These represent modifier keys that can be combined with other keys.
 */
typedef enum td_key_mod
{
    TD_MOD_NONE  = 0, /**< No modifier key */
    TD_MOD_SHIFT = 1, /**< Shift modifier key */
    TD_MOD_CTRL  = 2,  /**< Control modifier key */
    TD_MOD_ALT   = 4    /**< Alt modifier key */
} td_key_mod_t;

/**
 * @enum td_key_token_t
 * @brief Enumeration for key tokens.
 * 
 * These values represent various key codes for keyboard keys.
 */
typedef enum td_key_token
{
    TD_KEY_SPACE = 32,        /**< Space ' ' key */
    TD_KEY_ASTROPHE = 39,     /**< Apostrophe '\'' key */
    TD_KEY_COMMA = 44,        /**< Comma ',' key */
    TD_KEY_MINUS,             /**< Minus '-' key */
    TD_KEY_PERIOD,            /**< Period '.' key */
    TD_KEY_SLASH,             /**< Slash '/' key */
    TD_KEY_0,                 /**< '0' key */
    TD_KEY_1,                 /**< '1' key */
    TD_KEY_2,                 /**< '2' key */
    TD_KEY_3,                 /**< '3' key */
    TD_KEY_4,                 /**< '4' key */
    TD_KEY_5,                 /**< '5' key */
    TD_KEY_6,                 /**< '6' key */
    TD_KEY_7,                 /**< '7' key */
    TD_KEY_8,                 /**< '8' key */
    TD_KEY_9,                 /**< '9' key */
    TD_KEY_SEMICOLON = 59,    /**< Semicolon key */
    TD_KEY_EQUAL = 61,        /**< Equal key */
    TD_KEY_A = 65,            /**< 'A' key */
    TD_KEY_B,                 /**< 'B' key */
    TD_KEY_C,                 /**< 'C' key */
    TD_KEY_D,                 /**< 'D' key */
    TD_KEY_E,                 /**< 'E' key */
    TD_KEY_F,                 /**< 'F' key */
    TD_KEY_G,                 /**< 'G' key */
    TD_KEY_H,                 /**< 'H' key */
    TD_KEY_I,                 /**< 'I' key */
    TD_KEY_J,                 /**< 'J' key */
    TD_KEY_K,                 /**< 'K' key */
    TD_KEY_L,                 /**< 'L' key */
    TD_KEY_M,                 /**< 'M' key */
    TD_KEY_N,                 /**< 'N' key */
    TD_KEY_O,                 /**< 'O' key */
    TD_KEY_P,                 /**< 'P' key */
    TD_KEY_Q,                 /**< 'Q' key */
    TD_KEY_R,                 /**< 'R' key */
    TD_KEY_S,                 /**< 'S' key */
    TD_KEY_T,                 /**< 'T' key */
    TD_KEY_U,                 /**< 'U' key */
    TD_KEY_V,                 /**< 'V' key */
    TD_KEY_W,                 /**< 'W' key */
    TD_KEY_X,                 /**< 'X' key */
    TD_KEY_Y,                 /**< 'Y' key */
    TD_KEY_Z,                 /**< 'Z' key */
    TD_KEY_LEFT_BRACKET = 91, /**< Left bracket '[' */
    TD_KEY_BACKSLASH,         /**< Backslash '\\' */
    TD_KEY_RIGHT_BRACKET,     /**< Right bracket ']' */
    TD_KEY_GRAVE_ACCENT = 96, /**< Grave accent '`' */
    TD_KEY_ESCAPE = 256,      /**< Escape key */
    TD_KEY_ENTER,             /**< Enter key */
    TD_KEY_TAB,               /**< Tab key */
    TD_KEY_BACKSPACE,         /**< Backspace key */
    TD_KEY_INSERT,            /**< Insert key */
    TD_KEY_DELETE,            /**< Delete key */
    TD_KEY_RIGHT,             /**< Right arrow key */
    TD_KEY_LEFT,              /**< Left arrow key */
    TD_KEY_DOWN,              /**< Down arrow key */
    TD_KEY_UP,                /**< Up arrow key */
    TD_KEY_PAGE_UP,           /**< Page up key */
    TD_KEY_PAGE_DOWN,         /**< Page down key */
    TD_KEY_HOME,              /**< Home key */
    TD_KEY_END,               /**< End key */
    TD_KEY_F1 = 290,          /**< Function key F1 */
    TD_KEY_F2,                /**< Function key F2 */
    TD_KEY_F3,                /**< Function key F3 */
    TD_KEY_F4,                /**< Function key F4 */
    TD_KEY_F5,                /**< Function key F5 */
    TD_KEY_F6,                /**< Function key F6 */
    TD_KEY_F7,                /**< Function key F7 */
    TD_KEY_F8,                /**< Function key F8 */
    TD_KEY_F9,                /**< Function key F9 */
    TD_KEY_F10,               /**< Function key F10 */
    TD_KEY_F11,               /**< Function key F11 */
    TD_KEY_F12                /**< Function key F12 */
} td_key_token_t;

/**
 * @typedef td_key_callback_t
 * @brief Typedef for key event callback function.
 * 
 * This typedef defines the function pointer type for handling key events.
 * The callback function is called when a key event occurs, passing the key code,
 * action (press, release, or hold), and modifier keys
 */
typedef void (*td_key_callback_t)(
        td_window_t *window,
        td_key_token_t key, 
        td_key_action_t action, 
        td_key_mod_t mod
);

/**
 * @typedef td_mouse_button_callback
 * @brief Typedef for mouse button event callback function
 *
 * This typedef define the function pointer type for handling mouse button events.
 * The callback function will be invoked when mouse button was pressed,
 * passing the button code, action (press, release, or hold), and modifier keys
 */
typedef void (*td_mouse_button_callback_t)(
        td_window_t *window,
        int button, 
        td_key_action_t actions, 
        td_key_mod_t mods
);

typedef void (*td_cursor_pos_callback_t)(
        td_window_t *window,
        td_i32 x, 
        td_i32 y
);

TD_DEFINE_CALLBACK_SETTER(resize);
TD_DEFINE_CALLBACK_SETTER(key);
TD_DEFINE_CALLBACK_SETTER(mouse_button);
TD_DEFINE_CALLBACK_SETTER(cursor_pos);

td_error_t td_set_virtual_cursor_keybind(
        const td_key_token_t key,
        const td_key_mod_t mod);

td_error_t td_poll_events(void);

#endif
