#ifndef _CAMERA_H
#define _CAMERA_H

#include <stdint.h>
#include "camera_type.h"


extern struct camera_t *camera_open(const char *dev);


extern void camera_release(struct camera_t *p);


#endif
