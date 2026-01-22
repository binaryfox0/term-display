#ifndef TD_INPUT_H
#define TD_INPUT_H

#include "td_main.h"

typedef struct tdp_input_cb
{
    td_key_callback keycb;
    td_mouse_callback mousecb;
} tdp_input_cb;

#endif
