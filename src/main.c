#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>
#include "../include/camera.h"


uint8_t f_verbose = 0;

static uint8_t is_device_available(const char *path);

extern void print_v4l2_description(struct camera_t *p_camera);


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
    };


    int opt;
    char device_path[256];
    uint8_t f_info = 0;

    // 初期化
    memset((void *)device_path, 0, sizeof(device_path));
    snprintf(device_path, 256, "/dev/video0");


    while((opt = getopt_long(argc, argv, "d:IV", (const struct option *)opts, NULL)) > 0)
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
        }
    }

    if(strlen(device_path) == 0 || ! is_device_available(device_path))
        return -1;

    struct camera_t *p_camera = camera_open(device_path);
    if(p_camera == NULL)
        return -1;

    if(f_info)
    {
        print_v4l2_description(p_camera);
    }

    camera_release(p_camera);

    return 0;
}



static uint8_t is_device_available(const char *path)
{
    if(access(path, F_OK) != 0)
        return 0;

    struct stat stat;

    memset((void *)&stat, 0, sizeof(struct stat));

    if(lstat(path, &stat) != 0)
        return 0;

    return S_ISCHR(stat.st_mode) ? 1 : 0;
}
