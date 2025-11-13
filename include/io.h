#ifndef _IO_H
#define _IO_H


extern int32_t get_video_number(const char *path);


extern char *path_combine(
        size_t n,
        char separator,
        ...);


extern char *get_filename(
        const char *path,
        char separator);


extern char *get_filename_without_extensions(
        const char *path,
        char separator);


#endif
