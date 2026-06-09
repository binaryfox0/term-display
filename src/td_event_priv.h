#ifndef TD_EVENT_PRIV_H
#define TD_EVENT_PRIV_H

#include <td_def.h>
#include <td_event.h>

#define TDP_CALLBACK_NAME(type) type##_callback
#define TDP_DEFINE_CALLBACK_VAR(type) \
    td_##type##_callback_t TDP_CALLBACK_NAME(type)

typedef struct tdp_event_context tdp_event_context_t;
tdp_event_context_t *tdp_event_create_context(
        const td_ivec2 initial_cur_pos);
void tdp_event_destroy_context(
        tdp_event_context_t *context);

#endif
