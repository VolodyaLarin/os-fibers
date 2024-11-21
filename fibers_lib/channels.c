#include <pthread.h>
#include <string.h>

#include "channels.h"


#define CHANNEL_MAX_BUFFER_SIZE sizeof(int) * 1024

struct fl_channel_struct {
    pthread_mutex_t lock;

    char *start;
    char *end;
    char *read_ptr;
    char *write_ptr;
};

int fl_channel_create(fl_channel **ptr) {
    fl_channel *channel = malloc(sizeof(fl_channel));


    char *buffer = calloc(CHANNEL_MAX_BUFFER_SIZE, 1);

    channel->start = buffer;
    channel->end = buffer + CHANNEL_MAX_BUFFER_SIZE - 1;

    channel->write_ptr = buffer;
    channel->read_ptr = buffer;

    pthread_mutex_init(&channel->lock, NULL);

    *ptr = channel;
    return 0;
}

int fl_channel_write(fl_channel *chan, const char *data, size_t size) {
    if (!chan) {
        DEBUG_LOG0("channel is nil\n");
        return fiber_invalid_args;
    }
    if (!data) {
        DEBUG_LOG0("data is nil\n");
        return fiber_invalid_args;
    }


    pthread_mutex_lock(&chan->lock);

    size_t size1 = chan->end - chan->write_ptr;
    if (size1 > size + 1) {
        chan->write_ptr += size + 1;
        memcpy(chan->write_ptr, data, size);
    } else {
        memcpy(chan->write_ptr, data, size1);
        memcpy(chan->start, data + size1, size - size1);

        chan->write_ptr = chan->start + size - size1 + 1;
        chan->write_ptr[-1] = '\0';

        memcpy(chan->write_ptr, data, size);
    }

    chan->write_ptr[-1] = '\0';

    pthread_mutex_unlock(&chan->lock);

    return fiber_ok;
}



int fl_channel_read(fl_fiber *fiber, fl_channel *chan, char *data, size_t size) {
    if (!chan) {
        DEBUG_LOG0("channel is nil\n");
        return fiber_invalid_args;
    }
    if (!data) {
        DEBUG_LOG0("data is nil\n");
        return fiber_invalid_args;
    }


    pthread_mutex_lock(&chan->lock);



    pthread_mutex_unlock(&chan->lock);

    return fiber_ok;
}
