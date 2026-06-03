#ifndef TD_EVENT_PRIV_H
#define TD_EVENT_PRIV_H

#include <td_def.h>
#include <td_event.h>

#define TDP_CALLBACK_NAME(type) type##_callback
#define TDP_DEFINE_CALLBACK_VAR(type) \
    td_##type##_callback_t TDP_CALLBACK_NAME(type)
typedef struct tdp_event
{
    td_ivec2 term_size;
    TDP_DEFINE_CALLBACK_VAR(resize);
} tdp_event_t;

#endif
