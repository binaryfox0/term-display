#ifndef TD_WINDOW_H
#define TD_WINDOW_H

#include <td_error.h>


/*
 * @brief Terminal color rendering modes.
 *
 * Defines the available output color capabilities used by the renderer.
 * These modes control how colors are quantized and emitted to the target
 * terminal or display backend.
 */
typedef enum td_color_mode
{
    TD_COLOR_GRAYSCALE_24,   /**< 24-level grayscale output. */
    TD_COLOR_GRAYSCALE_256,  /**< 256-level grayscale output. */
    TD_COLOR_ANSI_216,       /**< 216-color ANSI palette output. */
    TD_COLOR_TRUECOLOR       /**< Full 24-bit RGB truecolor output. */
} td_color_mode_t;

/**
 * @brief Clear the terminal screen, not framebuffer
 */
td_error_t td_window_clear(void);

td_error_t td_window_poll(void);
td_error_t td_window_present(void);

#endif
