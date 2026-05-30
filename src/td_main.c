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

#include <td_main.h>
#include "td_priv.h"

#include "td_context.h"
#include "td_window_priv.h"
#include "td_renderer_priv.h"

const char *td_copyright_notice(void)
{
    return
        "Copyright (c) 2026 binaryfox0 (Duy Pham Duc)\n"
        "License under the MIT License (see LICENSE file)\n\n"
        "Extremely Fast Line Algorithm Var D (Addition Fixed Point)\n"
        "Copyright 2001, By Po-Han Lin\n"
        "Freely usable in non-commercial applications as long as \n"
        "credits to Po-Han Lin and a link to http://www.edepot.com \n"
        "is provided in source code and can be seen in compiled executable.\n"
        "Commercial applications please inquire about licensing the algorithms.";
}


#if defined(TD_PLATFORM_WINDOWS)
BOOL tdp_stop_handle(DWORD ctrl_type)
{
    switch(ctrl_type)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
            __display_is_running = td_false;
            return FALSE;
        default:
            return FALSE;
    }
    return FALSE;
}
#else
void tdp_stop_handle(int signal)
{
    (void) signal;
    __display_is_running = TD_FALSE;
}
#endif

struct {
    td_error_t (*init)(void);
    void (*exit)(void);
} tdp_susbsytems[] = {
    {tdp_context_init, tdp_context_exit},
    {tdp_window_init, tdp_window_exit},
    {tdp_renderer_init, tdp_renderer_exit}
};

td_error_t td_init(void)
{
    td_error_t err = TD_ERR_OK;
    if(tdp_ctx) 
        return TD_ERR_OK;

    for(td_i32 i = 0; i < 

    err = tdp_setup_env(tdp_stop_handle);
        return TD_;
    
    tdp_term_size = tdp_prev_size = tdp_get_termsz();      // Disable calling callback on the first time
    if(tdp_renderer_init(tdp_term_size))
        return TD_FALSE;
    __display_is_running = TD_TRUE;
    if(tdp_debug(init, ) == 1)
        return TD_FALSE;
    td_initialized = TD_TRUE;
    return TD_TRUE;
}


static td_resize_callback tdp_resize_callback = 0;
void td_set_resize_callback(td_resize_callback callback) {
    tdp_resize_callback = callback;
}

void td_quit(void)
{
    if(!td_initialized) return;
    tdp_renderer_exit();
    tdp_restore_env();
    tdp_debug(quit, );
    td_initialized = TD_FALSE;
}
