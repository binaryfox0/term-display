#include "td_event.h"

#include "td_context.h"
#include "td_term.h"
#include "td_window_priv.h"
#include "td_event_priv.h"

#define TDP_IMPLEMENT_CALLBACK_SETTER(type) \
    TD_DEFINE_CALLBACK_SETTER(type) \
    { \
        if(!window) \
            return TD_ERR_NOT_INITIALIZED; \
        window->event.TDP_CALLBACK_NAME(type) = callback; \
        return TD_ERR_OK; \
    }

TDP_IMPLEMENT_CALLBACK_SETTER(resize)
// TDP_IMPLEMENT_CALLBACK_SETTER(key)
// TDP_IMPLEMENT_CALLBACK_SETTER(mouse_button)
// TDP_IMPLEMENT_CALLBACK_SETTER(cursor_pos)

td_error_t td_poll_events(td_window_t *window)
{ 
    td_ivec2 current_size = {0};
    tdp_event_t *event = 0;
    if(!window)
        return TD_ERR_NOT_INITIALIZED;

    event = &window->event;
    current_size = tdp_term_get_size();
    if(
            current_size.x != event->term_size.x ||
            current_size.y != event->term_size.y)
    {
        if(event->TDP_CALLBACK_NAME(resize))
            event->TDP_CALLBACK_NAME(resize);
        if(window->flags & TD_WINDOW_RESIZABLE)
            ;  
        tdp_term_clear();
        event->term_size = current_size;
    }
    return TD_ERR_OK;
}

