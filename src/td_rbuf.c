#include "td_rbuf.h"

int tdp_rbuf_get(
        tdp_rbuf_t *rb,
        const int index)
{
    int end_index = 0;
    int n = 0;
    int phys = 0;
    int i = 0;
    char tmp[TDP_RBUF_SIZE] = {0};

    if (rb->count == 0)
    {
        n = rb->read_cb(
                rb->userdata,
                tmp,
                TDP_RBUF_SIZE);

        if (n <= 0)
            return -1;

        for (i = 0; i < n; i++)
            rb->buffer[i] = tmp[i];

        rb->first_index = index;
        rb->head = 0;
        rb->count = n;

        return (unsigned char)rb->buffer[0];
    }

    if (index < rb->first_index)
        return -1;

    end_index = rb->first_index + rb->count - 1;

    while (index > end_index)
    {
        n = rb->read_cb(
                rb->userdata,
                tmp,
                TDP_RBUF_SIZE);

        if (n <= 0)
            return -1;

        for (i = 0; i < n; i++)
        {
            int tail = 0;

            tail = (rb->head + rb->count) % TDP_RBUF_SIZE;

            if (rb->count == TDP_RBUF_SIZE)
            {
                rb->head = (rb->head + 1) % TDP_RBUF_SIZE;
                rb->first_index++;
                rb->count--;
            }

            rb->buffer[tail] = tmp[i];
            rb->count++;
        }

        end_index = rb->first_index + rb->count - 1;
    }

    phys = (rb->head + (index - rb->first_index)) % TDP_RBUF_SIZE;

    return (unsigned char)rb->buffer[phys];
}
