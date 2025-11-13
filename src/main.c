#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include "../include/camera.h"
#include "../include/io.h"


uint8_t f_verbose = 0;

static uint8_t f_exit = 0;
static uint8_t is_device_available(const char *path);

extern void print_v4l2_description(struct camera_t *p_camera);


/**
 * GPIO 監視スレッド関数
 */
static void *gpio_monitor_routine(void *arg)
{
    do
    {
        fprintf(stderr, "Interval...\n");
        sleep(1);
    }
    while(! f_exit);

    return NULL;
}


/**
 * メイン関数
 */
int main(
        int argc,
        char *argv[])
{
    struct option opts[] =
    {
        { "device", required_argument, NULL, 'd' },
        { "info", no_argument, NULL, 'I' },
        { "verbose", no_argument, NULL, 0 },
        { "help", no_argument, NULL, 'h' },
    };


    int opt;
    char device_path[256];
    uint8_t f_info = 0;

    // 初期化
    memset((void *)device_path, 0, sizeof(device_path));
    snprintf(device_path, 256, "/dev/video0");


    int longint;
    while((opt = getopt_long(argc, argv, "d:IVh", (const struct option *)opts, &longint)) > 0)
    {
        switch(opt)
        {
            case 'd':
                if(optarg != NULL)
                    strncpy(device_path, optarg, 256);
                break;

            case 'I':
                f_info = 1;
                break;

            case 'V':
                f_verbose = 1;
                break;

            case 'h':
                break;
        }
    }

    fprintf(stderr, "%s\n", device_path);

    if(strlen(device_path) == 0 || ! is_device_available(device_path))
        return -1;

    struct camera_t *p_camera = camera_open(device_path);
    if(p_camera == NULL)
        return -1;

    if(f_info)
    {
        print_v4l2_description(p_camera);
        camera_release(p_camera);
        return 0;
    }


    pthread_t thread;
    if(pthread_create(&thread, NULL, gpio_monitor_routine, NULL) != 0)
    {
        camera_release(p_camera);
        return -1;
    }

    camera_set_properties(p_camera);

    camera_set_request_buffers(p_camera, 3);

    camera_buffer_enqueue(p_camera);

    camera_set_stream_state(p_camera, V4L2_BUF_TYPE_VIDEO_CAPTURE, 1);

    camera_poll(p_camera);

    camera_dequeue(p_camera);

    camera_write_buffer(p_camera, "dump");

    fprintf(stdout, "Please Any Keys...\n");
    getchar();

    camera_set_stream_state(p_camera, V4L2_BUF_TYPE_VIDEO_CAPTURE, 0);

    f_exit = 1;
    camera_release(p_camera);

    pthread_join(thread, NULL);

    return 0;
}



static uint8_t is_device_available(const char *path)
{
    if(access(path, F_OK) != 0)
        return 0;

    struct stat statbuf;

    memset((void *)&statbuf, 0, sizeof(struct stat));

    if(stat(path, &statbuf) != 0)
        return 0;

    return S_ISCHR(statbuf.st_mode) ? 1 : 0;
}
