#ifndef TD_TERM_H
#define TD_TERM_H

#include <td_def.h>
#include <td_error.h>

#if defined(TD_PLATFORM_WINDOWS)
#   include <windows.h>
    typedef BOOL (*tdp_sighand)(DWORD);
#elif defined(TD_PLATFORM_UNIX)
    typedef void (*tdp_sighand_t)(int);
#else
#   error "term-display haven't added support for this platform"
#endif

td_error_t tdp_term_init(void);
void tdp_term_set_stop_handle(const tdp_sighand_t handle);
td_ivec2 tdp_term_get_size(void);
void tdp_term_clear(void);
td_bool tdp_term_stdin_ready(const int ms);
int tdp_term_stdin_available(void);
td_error_t tdp_term_toggle_stop(const td_bool enable);
void tdp_term_exit(void);

#endif

