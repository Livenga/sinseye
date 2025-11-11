#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>



int32_t xioctl(int fd, int request, void *arg)
{
    int32_t ret;

    do
    {
        ret = ioctl(fd, request, arg);
    }
    while(ret == EOF && EINTR == errno);

    return ret;
}
