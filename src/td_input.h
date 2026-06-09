#ifndef TD_INPUT_H
#define TD_INPUT_H

#include <td_event.h>
#include "td_event_priv.h"

void tdp_poll_input(
        td_window_t *window,
        TDP_DEFINE_CALLBACK_VAR(key),
        TDP_DEFINE_CALLBACK_VAR(mouse_button),
        TDP_DEFINE_CALLBACK_VAR(cursor_pos)
);

#endif
