#ifndef TD_OPTION_H
#define TD_OPTION_H

#include <td_def.h>
#include <td_error.h>

 /**
  * @enum td_option_t
  * @brief Enumeration for term-display settings.
  * 
  * These settings allow configuration of various aspects of the term-display, 
  * including size, depth buffer, and other display properties.
  */
typedef enum td_option 
{
    TD_OPT_AUTO_RESIZE = 0,             /**< Automatic resizing of the display */
    TD_OPT_PIXEL_WIDTH,                 /**< Pixel width of the display */
    TD_OPT_PIXEL_HEIGHT,                /**< Pixel height of the display */
    TD_OPT_COLOR_MODE,                /**< Display mode (grayscale, truecolor, etc.) */
    TD_OPT_DISPLAY_ROTATION,            /**< Rotation of the display */
    TD_OPT_DEPTH_BUFFER,                /**< Enable or disable depth buffer */
    TD_OPT_DISABLE_STOP_SIG,            /**< Disable stop signal (SIGINT, SIGSTOP, etc) for the display */
    TD_OPT_SHIFT_TRANSLATE,             /**< Option to shift and translate the display */
    TD_OPT_WINDOW_SIZE,                /**< Option for the display size */
    TD_OPT_WINDOW_POS,                 /**< The placement position of display (cells) */
    __TD_OPT_MAX__
} td_option_t;

/**
 * @brief Gets or sets the value of a term-display setting.
 * 
 * @param type The setting to query/modify.
 * @param get A boolean indicating whether to get or set the setting.
 * @param option A pointer to the setting value to retrieve or set.
 * 
 * @return A boolean indicating success or failure.
 */
td_error_t td_option(
        const td_option_t opt, 
        const td_bool get, 
        void *ptr);

#endif
