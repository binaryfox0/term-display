#include <td_event.h>

#include <stdio.h>
#include <stdlib.h>

#include "td_context.h"
#include "td_term.h"
#include "td_window_priv.h"
#include "td_event_priv.h"
#include "td_input.h"

#define TDP_IMPLEMENT_CALLBACK_SETTER(type) \
    TD_DEFINE_CALLBACK_SETTER(type) \
    { \
        if(!window) \
            return TD_ERR_NOT_INITIALIZED; \
        window->event->TDP_CALLBACK_NAME(type) = callback; \
        return TD_ERR_OK; \
    }

typedef struct tdp_keybind
{
    td_key_token_t key;
    td_key_mod_t mod;
} tdp_keybind_t;

typedef struct tdp_virtual_cursor
{
    tdp_keybind_t active_kbind;
    td_bool enabled;
    td_ivec2 cur_pos;
} tdp_virtual_cursor_t;


typedef struct tdp_event_context
{
    td_ivec2 term_size;
    tdp_keybind_t exit_kbind;
    tdp_virtual_cursor_t virt_cur;
    TDP_DEFINE_CALLBACK_VAR(resize);
    TDP_DEFINE_CALLBACK_VAR(key);
    TDP_DEFINE_CALLBACK_VAR(mouse_button);
    TDP_DEFINE_CALLBACK_VAR(cursor_pos);
} tdp_event_context_t;

TDP_IMPLEMENT_CALLBACK_SETTER(resize)
TDP_IMPLEMENT_CALLBACK_SETTER(key)
// TDP_IMPLEMENT_CALLBACK_SETTER(mouse_button)
// TDP_IMPLEMENT_CALLBACK_SETTER(cursor_pos)

static void tdp_mitm_key_callback(
        td_window_t *window,
        td_key_token_t key, 
        td_key_action_t action, 
        td_key_mod_t mod
)
{
    tdp_event_context_t *event = 0;
    tdp_virtual_cursor_t *virt_cur = 0;
    event = window->event;
    virt_cur = &event->virt_cur;

    if(
            event->exit_kbind.key == key &&
            event->exit_kbind.mod == mod)
    {
        window->should_close = TD_TRUE;
        return;
    }

    if(
            event->TDP_CALLBACK_NAME(cursor_pos) &&
            virt_cur->active_kbind.key == key &&
            virt_cur->active_kbind.mod == mod &&
            action == TD_ACTION_PRESS
    )
    {
        event->virt_cur.enabled = !event->virt_cur.enabled;
        TDP_DEBUG_LOG("cursor emulation was %s", 
                event->virt_cur.enabled ? "enabled" : "disabled");
        return;
    }

    if(virt_cur->enabled)
    {
        switch(key)
        {
            case TD_KEY_UP:
                virt_cur->cur_pos.y--;
                break;

            case TD_KEY_DOWN:
                virt_cur->cur_pos.y++;
                break;

            case TD_KEY_LEFT:
                virt_cur->cur_pos.x--;
                break;

            case TD_KEY_RIGHT:
                virt_cur->cur_pos.x++;
                break;

            default:
                return;
        }
        event->TDP_CALLBACK_NAME(cursor_pos)(window, 
                virt_cur->cur_pos.x, virt_cur->cur_pos.y);
    }
    else
        event->TDP_CALLBACK_NAME(key)(window, key, action, mod);

}

td_error_t td_set_virtual_cursor_keybind(
        const td_key_token_t key,
        const td_key_mod_t mod)
{
    tdp_event_context_t *event = 0;
    if(!tdp_ctx || !tdp_ctx->window)
        return TD_ERR_NOT_INITIALIZED;
    event = tdp_ctx->window->event;
    event->virt_cur.active_kbind.key = key;
    event->virt_cur.active_kbind.mod = mod;
    return TD_ERR_OK;
}

td_error_t td_poll_events(void)
{ 
    td_ivec2 current_size = {0};
    td_window_t *window = {0};
    tdp_event_context_t *event = 0;
    if(!tdp_ctx)
        return TD_ERR_NOT_INITIALIZED;

    if(!tdp_ctx->window)
        return TD_ERR_OK;

    window = tdp_ctx->window;
    event = window->event;
    current_size = tdp_term_get_size();
    if(
            current_size.x != event->term_size.x ||
            current_size.y != event->term_size.y)
    {
        if(event->TDP_CALLBACK_NAME(resize))
            event->TDP_CALLBACK_NAME(resize);
        if(window->flags & TD_WINDOW_RESIZABLE)
        {
            td_ivec2 logical_size = {0};
            td_ivec2 rotated_size = {0};

            logical_size = tdp_calculate_logical(window, current_size);
            rotated_size = tdp_apply_rotation(window, logical_size);
            td_window_resize(window, rotated_size);  
        }
        tdp_term_clear();
        tdp_window_update_bound(window);
        event->term_size = current_size;
    }

    tdp_poll_input(
        window,
        tdp_mitm_key_callback,
        0,
        0
    );
    return TD_ERR_OK;
}

tdp_event_context_t *tdp_event_create_context(
        const td_ivec2 initial_cur_pos)
{
    tdp_event_context_t *context = 0;
    context = calloc(1, sizeof(*context));
    if(!context)
        return 0;
    context->term_size = tdp_term_get_size();
    context->virt_cur.cur_pos = initial_cur_pos;
    context->exit_kbind.mod = TD_MOD_CTRL;
    context->exit_kbind.key = TD_KEY_C;
    return context;
}

void tdp_event_destroy_context(
        tdp_event_context_t *context)
{
    free(context);
}
