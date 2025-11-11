#ifndef _CAMERA_H
#define _CAMERA_H

#include <stdint.h>
#include "camera_type.h"


extern struct camera_t *camera_open(const char *dev);


/**
 */
extern int32_t camera_set_request_buffers(
        struct camera_t *p_cam,
        int32_t buffer_count);



/**
 */
extern int32_t camera_set_stream_state(
        struct camera_t *p_cam,
        enum v4l2_buf_type type,
        uint8_t  state);


/**
 */
extern int32_t camera_set_properties(struct camera_t *p_cam);



/**
 */
extern int32_t camera_buffer_enqueue(struct camera_t *p_cam);



/**
 */
extern void camera_poll(struct camera_t *p_cam);



/**
 */
extern int32_t camera_dequeue(struct camera_t *p_cam);



/**
 */
extern int32_t camera_write_buffer(
        struct camera_t *p_cam,
        const char *path);



/**
 */
extern void camera_release(struct camera_t *p);


#endif
