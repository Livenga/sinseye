#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/camera_type.h"



extern int32_t xioctl(int fd, int request, void *arg);


/**
*/
struct camera_t *camera_open(const char *dev)
{
    int32_t fd = open(dev, O_RDWR);
    if(fd < 0)
    {
        return NULL;
    }

    struct camera_t *p = (struct camera_t *)calloc(1, sizeof(struct camera_t));

    p->path = (const char *)dev;
    p->fd   = fd;


    struct v4l2_capability cap;
    memset((void *)&cap, 0, sizeof(struct v4l2_capability));

    int32_t ret = xioctl(fd, VIDIOC_QUERYCAP, &cap);

    if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0 ||
            (cap.capabilities & V4L2_CAP_STREAMING) == 0)
        goto not_supported;

    return p;


not_supported:
    fprintf(stderr, "\"%s\" はキャプチャーとストリーミングをサポートしていません。\n", dev);

    close(p->fd);
    free(p);

    return NULL;
}



/**
*/
void camera_release(struct camera_t *p)
{
    close(p->fd);
    free(p);
}
