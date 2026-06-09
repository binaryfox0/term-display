#ifndef TD_RBUF_H
#define TD_RBUF_H

#define TDP_RBUF_SIZE 256

typedef int (*tdp_rbuf_read_callback_t)(
        void *userdata,
        char *buffer,
        int size);

typedef struct tdp_rbuf
{
    char buffer[TDP_RBUF_SIZE];

    int first_index;
    int head;
    int count;

    tdp_rbuf_read_callback_t read_cb;
    void *userdata;
} tdp_rbuf_t;

int tdp_rbuf_get(
        tdp_rbuf_t *rb,
        const int index);

#endif
