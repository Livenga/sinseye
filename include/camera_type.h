#ifndef _CAMERA_TYPE_H
#define _CAMERA_TYPE_H

#include <stdint.h>


/**
 */
struct stream_buffer_t
{
    void *m;
    size_t length;
};



struct camera_t
{
    const char *path;
    int fd;

    int32_t buffer_count;
    struct stream_buffer_t *buffers;
};

#endif
