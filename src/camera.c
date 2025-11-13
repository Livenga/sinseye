#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <regex.h>
#include "../include/io.h"
#include "../include/camera_type.h"



extern int32_t xioctl(int fd, int request, void *arg);


static char **find_video_device_paths(size_t *p_count)
{
    const char *p_root = "/dev";

    struct dirent *p_dirent = NULL;
    DIR *p_dir = opendir(p_root);
    if(p_dir == NULL)
    {
        return NULL;
    }

    regex_t reg;
    regmatch_t pmatch[1];
    memset((void *)&reg, 0, sizeof(regex_t));
    int state = regcomp(&reg, "^video[0-9]+$", REG_EXTENDED);

    if(state != 0)
    {
        closedir(p_dir);
        return NULL;
    }


    size_t cnt = 0;
    while((p_dirent = readdir(p_dir)) != NULL)
    {
        if(*(p_dirent->d_name + 0) == '.')
            continue;

        state = regexec(&reg, p_dirent->d_name, 1, pmatch, 0);
        if(state == 0)
            ++cnt;
    }
    closedir(p_dir);

    if(p_count != NULL)
        *p_count = cnt;


    p_dir = opendir(p_root);
    if(p_dir == NULL)
    {
        return NULL;
    }

    int cursor = 0;
    char **p_ret = (char **)calloc(cnt, sizeof(char *));
    if(p_ret == NULL)
    {
        closedir(p_dir);
        return NULL;
    }

    while((p_dirent = readdir(p_dir)) != NULL)
    {
        if(*(p_dirent->d_name + 0) == '.')
            continue;

        state = regexec(&reg, p_dirent->d_name, 1, pmatch, 0);
        if(state == 0 && cursor < cnt)
        {
            *(p_ret + cursor++) = path_combine(2, '/', p_root, p_dirent->d_name);
        }
    }

    closedir(p_dir);

    return p_ret;
}



/**
 */
static int8_t is_available_device(const char *path)
{
    int fd = open(path, O_RDWR);
    if(fd <= 0)
        return 0;

    struct v4l2_capability cap;
    memset((void *)&cap, 0, sizeof(struct v4l2_capability));

    int ret = xioctl(fd, VIDIOC_QUERYCAP, (void *)&cap);
    if(ret != 0)
    {
        close(fd);
        return 0;
    }

    close(fd);

    if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) > 0 &&
            (cap.capabilities & V4L2_CAP_STREAMING) > 0 &&
            (cap.device_caps & V4L2_CAP_META_CAPTURE) == 0)
    {
        return 1;
    }

    return 0;
}



/**
 * 使用可能なデバイス一覧の取得
 * /dev/video[0-9]+ の正規表現に一致したデバイスでキャプチャ及びストリーミングを対応し、メタデータキャプチャが非対応のデバイスパスを列挙。
 *
 * @param p_count 検出したデバイスの件数を格納するポイント
 * @return 使用可能なデバイス一覧
 */
char **find_available_devices(size_t *p_count)
{
    size_t count = 0;
    char **detected_device_paths = find_video_device_paths(&count);


    char **p_ret = (char **)calloc(count, sizeof(char *));

    size_t cursor = 0;
    for(int i = 0; i < count; ++i)
    {
        char *path = *(detected_device_paths + i);
        if(path == NULL)
            continue;

        int8_t is_available = is_available_device(path);
        if(is_available)
        {
            *(p_ret + cursor) = *(detected_device_paths + i);
            ++cursor;
        }
        else
        {
            free(*(detected_device_paths + i));
        }
    }

    if(p_count != NULL)
        *p_count = cursor;

    if(cursor != count)
    {
        p_ret = realloc(p_ret, cursor);
    }

    free(detected_device_paths);

    return p_ret;
}




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
int32_t camera_set_request_buffers(
        struct camera_t *p_cam,
        int32_t buffer_count)
{
    struct v4l2_requestbuffers req;
    memset((void *)&req, 0, sizeof(struct v4l2_requestbuffers));

    req.count  = buffer_count;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if(xioctl(p_cam->fd, VIDIOC_REQBUFS, (void *)&req) == -1)
        return -1;

    p_cam->buffer_count = req.count;

    if(req.count < buffer_count)
    {
    }

    if(req.count <= 0)
    {
        p_cam->buffers = NULL;
        return -1;
    }


    p_cam->buffers = (struct stream_buffer_t *)calloc(req.count, sizeof(struct stream_buffer_t));


    for(int32_t i = 0; i < req.count; ++i)
    {
        struct v4l2_buffer vbuf;

        memset((void *)&vbuf, 0, sizeof(struct v4l2_buffer));
        vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vbuf.memory = V4L2_MEMORY_MMAP;
        vbuf.index  = i;

        if(xioctl(p_cam->fd, VIDIOC_QUERYBUF, (void *)&vbuf) == -1)
        {
            return -1;
        }

#ifdef __DEBUG__
        fprintf(stderr, "offset: %d, length: %d\n", vbuf.m.offset, vbuf.length);
#endif

        (p_cam->buffers + i)->m      = mmap(
                NULL, vbuf.length,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                p_cam->fd, vbuf.m.offset);
        (p_cam->buffers + i)->length = vbuf.length;

        if((p_cam->buffers + i)->m == NULL)
        {
            (p_cam->buffers + i)->length = -1;
            break;
        }
    }



    return 0;
}



/**
 */
int32_t camera_buffer_enqueue(struct camera_t *p_cam)
{
    for(int32_t i = 0; i < p_cam->buffer_count; ++i)
    {
        struct v4l2_buffer vbuf;
        memset((void *)&vbuf, 0, sizeof(struct v4l2_buffer));

        vbuf.index  = i;
        vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vbuf.memory = V4L2_MEMORY_MMAP;

        if(xioctl(p_cam->fd, VIDIOC_QBUF, (void *)&vbuf) == -1)
        {
            return -1;
        }
    }

    return 0;
}



/**
 */
int32_t camera_set_stream_state(
        struct camera_t *p_cam,
        enum v4l2_buf_type type,
        uint8_t  state)
{
    if(xioctl(p_cam->fd, VIDIOC_STREAMON, (void *)&type) == -1)
    {
        return -1;
    }

    return 0;
}



/**
 */
int32_t camera_set_properties(struct camera_t *p_cam)
{
    struct v4l2_format fmt;
    memset((void *)&fmt, 0, sizeof(struct v4l2_format));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    fmt.fmt.pix.width       = 1920;
    fmt.fmt.pix.height      = 1080;
    //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field       = V4L2_FIELD_ANY;


    if(xioctl(p_cam->fd, VIDIOC_S_FMT, &fmt) == -1)
        return -1;


    struct v4l2_format current_format;
    memset((void *)&current_format, 0, sizeof(struct v4l2_format));

    if(xioctl(p_cam->fd, VIDIOC_G_FMT, (void *)&current_format) == -1)
        return -2;

    if(fmt.fmt.pix.width != current_format.fmt.pix.width ||
            fmt.fmt.pix.height != current_format.fmt.pix.height)
        return -3;


    return 0;
}



void camera_poll(struct camera_t *p_cam)
{
    struct pollfd fds[1];
    fds[0].fd = p_cam->fd;
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, 5000);
    if(ret < 0)
        return;
}




/**
 */
int32_t camera_dequeue(struct camera_t *p_cam)
{
    struct v4l2_buffer vbuf;
    memset((void *)&vbuf, 0, sizeof(struct v4l2_buffer));

    vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vbuf.memory = V4L2_MEMORY_MMAP;


    if(xioctl(p_cam->fd, VIDIOC_DQBUF, (void *)&vbuf) == -1)
        return -1;

    return 0;
}




int32_t camera_write_buffer(
        struct camera_t *p_cam,
        const char *path)
{
    int fd = open("dump", O_CREAT | O_WRONLY, 0644);

    write(fd, (const void *)(p_cam->buffers + 0)->m, (p_cam->buffers + 0)->length);

    close(fd);
    return 0;
}




/**
*/
void camera_release(struct camera_t *p)
{
    if(p->buffers != NULL)
    {
        for(int i = 0; i < p->buffer_count; ++i)
        {
            if((p->buffers + i)->m == NULL ||
                    (p->buffers + i)->length <= 0)
                break;

            (p->buffers + i)->length = 0;
            munmap((p->buffers +i)->m, (p->buffers + i)->length);
        }
    }

    close(p->fd);
    free(p);
}
