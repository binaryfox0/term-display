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
#include "td_utils.h"
#include "td_term.h"
#include "td_context.h"

const char *td_copyright_notice(void)
{
    return
        "Copyright (c) 2026 binaryfox0\n"
        "License under the MIT License (see LICENSE file)\n\n"
        "Extremely Fast Line Algorithm Var D (Addition Fixed Point)\n"
        "Copyright 2001, By Po-Han Lin\n"
        "Freely usable in non-commercial applications as long as \n"
        "credits to Po-Han Lin and a link to http://www.edepot.com \n"
        "is provided in source code and can be seen in compiled executable.\n"
        "Commercial applications please inquire about licensing the algorithms.";
}

struct {
    td_error_t (*init)(void);
    void (*exit)(void);
} tdp_susbsytems[] = {
    {0, tdp_debug_exit},
};
td_error_t td_init(void)
{
    td_error_t err = TD_ERR_OK;
    if(tdp_ctx) 
        return TD_ERR_OK;

    err = tdp_context_init();
    if(err != TD_ERR_OK)
        return err;

    for(td_i32 i = 0; i < (td_i32)TDP_ARRSZ(tdp_susbsytems); i++)
    {
        if(tdp_susbsytems[i].init)
            err = tdp_susbsytems[i].init();
        if(err != TD_ERR_OK)
        {
            for(; i >= 0; i--)
                tdp_susbsytems[i].exit();
            tdp_context_exit();
            return err;
        }
    }

    tdp_term_init();
    tdp_ctx->init_ts = td_get_performance_counter();

    return TD_ERR_OK;
}

void td_quit(void)
{
    tdp_term_exit(); 
    for(td_i32 i = 0; i < (td_i32)TDP_ARRSZ(tdp_susbsytems); i++)
        tdp_susbsytems[i].exit();
    tdp_context_exit();
}
