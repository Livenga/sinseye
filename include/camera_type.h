#ifndef _CAMERA_TYPE_H
#define _CAMERA_TYPE_H

#include <stdint.h>


struct camera_t
{
    const char *path;
    int fd;
};

#endif
