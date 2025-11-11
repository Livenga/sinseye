#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>
#include "../include/camera_type.h"
#include "../include/value_description_type.h"


extern int32_t xioctl(int fd, int request, void *arg);

static char _s_now[20];
static char _s_fourcc[5];

static struct value_description_t v4l2_capability_descriptions [] = {
    { "V4L2_CAP_VIDEO_CAPTURE", V4L2_CAP_VIDEO_CAPTURE, "Is a video capture device" },
    { "V4L2_CAP_VIDEO_OUTPUT", V4L2_CAP_VIDEO_OUTPUT, "Is a video output device" },
    { "V4L2_CAP_VIDEO_OVERLAY", V4L2_CAP_VIDEO_OVERLAY, "Can do video overlay" },
    { "V4L2_CAP_VBI_CAPTURE", V4L2_CAP_VBI_CAPTURE, "Is a raw VBI capture device" },
    { "V4L2_CAP_VBI_OUTPUT", V4L2_CAP_VBI_OUTPUT, "Is a raw VBI output device" },
    { "V4L2_CAP_SLICED_VBI_CAPTURE", V4L2_CAP_SLICED_VBI_CAPTURE, "Is a sliced VBI capture device" },
    { "V4L2_CAP_SLICED_VBI_OUTPUT", V4L2_CAP_SLICED_VBI_OUTPUT, "Is a sliced VBI output device" },
    { "V4L2_CAP_RDS_CAPTURE", V4L2_CAP_RDS_CAPTURE, "RDS data capture" },
    { "V4L2_CAP_VIDEO_OUTPUT_OVERLAY", V4L2_CAP_VIDEO_OUTPUT_OVERLAY, "Can do video output overlay" },
    { "V4L2_CAP_HW_FREQ_SEEK", V4L2_CAP_HW_FREQ_SEEK, "Can do hardware frequency seek " },
    { "V4L2_CAP_RDS_OUTPUT", V4L2_CAP_RDS_OUTPUT, "Is an RDS encoder" },
    { "V4L2_CAP_VIDEO_CAPTURE_MPLANE", V4L2_CAP_VIDEO_CAPTURE_MPLANE, NULL },
    { "V4L2_CAP_VIDEO_OUTPUT_MPLANE", V4L2_CAP_VIDEO_OUTPUT_MPLANE, NULL },
    { "V4L2_CAP_VIDEO_M2M_MPLANE", V4L2_CAP_VIDEO_M2M_MPLANE, NULL },
    { "V4L2_CAP_VIDEO_M2M", V4L2_CAP_VIDEO_M2M, NULL },
    { "V4L2_CAP_TUNER", V4L2_CAP_TUNER, "has a tuner" },
    { "V4L2_CAP_AUDIO", V4L2_CAP_AUDIO, "has audio support" },
    { "V4L2_CAP_RADIO", V4L2_CAP_RADIO, "is a radio device" },
    { "V4L2_CAP_MODULATOR", V4L2_CAP_MODULATOR, "has a modulator" },
    { "V4L2_CAP_SDR_CAPTURE", V4L2_CAP_SDR_CAPTURE, "Is a SDR capture device" },
    { "V4L2_CAP_EXT_PIX_FORMAT", V4L2_CAP_EXT_PIX_FORMAT, "Supports the extended pixel format" },
    { "V4L2_CAP_SDR_OUTPUT", V4L2_CAP_SDR_OUTPUT, "Is a SDR output device" },
    { "V4L2_CAP_META_CAPTURE", V4L2_CAP_META_CAPTURE, "Is a metadata capture device" },
    { "V4L2_CAP_READWRITE", V4L2_CAP_READWRITE, "read/write systemcalls" },
    { "V4L2_CAP_EDID", V4L2_CAP_EDID, "Is an EDID-only device" },
    { "V4L2_CAP_STREAMING", V4L2_CAP_STREAMING, "streaming I/O ioctls" },
    { "V4L2_CAP_META_OUTPUT", V4L2_CAP_META_OUTPUT, "Is a metadata output device" },
    { "V4L2_CAP_TOUCH", V4L2_CAP_TOUCH, "Is a touch device" },
    { "V4L2_CAP_IO_MC", V4L2_CAP_IO_MC, "Is input/output controlled by the media controller" },
    { "V4L2_CAP_DEVICE_CAPS", V4L2_CAP_DEVICE_CAPS, "sets device capabilities field" },
    { NULL, 0, NULL },
};



static char *get_now()
{
    const time_t now = time(NULL);
    struct tm *p_tm = localtime(&now);

    memset((void *)_s_now, 0, sizeof(_s_now));

    snprintf(_s_now, 20, "%04d-%02d-%02dT%02d:%02d:%02d",
            1900 + p_tm->tm_year, 1 + p_tm->tm_mon, p_tm->tm_mday,
            p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);

    return _s_now;
}


static char *fourcc_to_string(uint32_t fourcc)
{
    memset((void *)_s_fourcc, 0, sizeof(_s_fourcc));

    *(_s_fourcc + 0) = fourcc         & 0xff;
    *(_s_fourcc + 1) = (fourcc >> 8)  & 0xff;
    *(_s_fourcc + 2) = (fourcc >> 16) & 0xff;
    *(_s_fourcc + 3) = (fourcc >> 24) & 0xff;

    return _s_fourcc;
}




/**
 */
static void print_v4l2_supported_capabilities(int fd)
{
    struct v4l2_capability cap;
    memset((void *)&cap, 0, sizeof(struct v4l2_capability));

    int32_t ret = xioctl(fd, VIDIOC_QUERYCAP, &cap);
    if(ret == -1)
        return;

    struct value_description_t *p_desc = v4l2_capability_descriptions;

    fprintf(stderr, "[%s] ===== V4L2 Supported Capabilities =====\n", get_now());

    do
    {
        if((cap.capabilities & p_desc->value) > 0)
        {
            fprintf(stderr, "\t0x%08lX  %s:\t%s\n", p_desc->value, p_desc->name, p_desc->description);
        }
    } while((++p_desc)->name != NULL);

    fprintf(stderr, "\n");
}



/**
 */
static void print_v4l2_frame_sizes(
        int fd,
        uint32_t pixel_format)
{
    struct v4l2_frmsizeenum frm;

    memset((void *)&frm, 0, sizeof(struct v4l2_frmsizeenum));
    fprintf(stderr, "\tFrame Sizes\n");

    int32_t i = 0;
    for(;;)
    {
        frm.index = i++;
        frm.pixel_format = pixel_format;

        int32_t ret = xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frm);
        if(ret == -1)
            break;


        // Interval 取得
        struct v4l2_frmivalenum frm_interval;
        memset((void *)&frm_interval, 0, sizeof(struct v4l2_frmivalenum));

        frm_interval.pixel_format = pixel_format;
        frm_interval.width        = frm.discrete.width;
        frm_interval.height       = frm.discrete.height;
        frm_interval.type         = frm.type;

        ret = xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frm_interval);

        if(ret != -1)
        {
            fprintf(stderr, "\t\t%2d %dx%d\t(%u / %u)\n",
                    i,
                    frm.discrete.width,
                    frm.discrete.height,
                    frm_interval.discrete.numerator,
                    frm_interval.discrete.denominator);
        }
        else
        {
            fprintf(stderr, "\t\t%2d %dx%d\n",
                    i,
                    frm.discrete.width,
                    frm.discrete.height);
        }
    }
}



/**
*/
static void print_v4l2_supported_formats(int fd)
{
    struct v4l2_fmtdesc fmt;

    memset((void *)&fmt, 0, sizeof(struct v4l2_fmtdesc));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    fprintf(stderr, "[%s] ===== V4L2 Supported Formats =====\n",
            get_now());

    int i = 0;
    for(;;)
    {
        fmt.index = i;

        int32_t ret = xioctl(fd, VIDIOC_ENUM_FMT, &fmt);
        if(ret == -1)
            break;

        fprintf(stderr, "\t#%d %s %s(0x%X)\n",
                i,
                fourcc_to_string(fmt.pixelformat),
                fmt.description,
                fmt.pixelformat);

        print_v4l2_frame_sizes(fd, fmt.pixelformat);

        ++i;
    }

    fprintf(stderr, "\n");
}



/**
*/
void print_v4l2_description(struct camera_t *p_camera)
{
    print_v4l2_supported_capabilities(p_camera->fd);
    print_v4l2_supported_formats(p_camera->fd);
}
