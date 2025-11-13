#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <regex.h>


static char *alloc_copy(const char *p);



/**
 */
int32_t get_video_number(const char *path)
{
    regex_t reg;
    memset((void *)&reg, 0, sizeof(regex_t));

    //int ret = regcomp(&reg, "video([0-9]+)", REG_EXTENDED);
    int ret = regcomp(&reg, "video([0-9]+)", REG_EXTENDED);

    if(ret != 0)
        return -1;


    regmatch_t matches[3];
    ret = regexec(&reg, path, 3, matches, 0);

    if(ret != 0)
        return -1;

    if(matches[1].rm_so == -1 || matches[1].rm_eo == -1)
        return -1;

    char *p = NULL;
    int32_t number = strtod(path + matches[1].rm_so, &p);

    if(strlen(path) > (p - path))
        return -1;

    return number;
}



/**
 */
char *path_combine(
        size_t n,
        char separator,
        ...)
{
    va_list ap, ap2;

    va_start(ap, separator);
    va_copy(ap2, ap);

    size_t len = 0;
    for(int i = 0; i < n; ++i)
    {
        char *arg = va_arg(ap, char *);
        len += strlen(arg) + 1;
    }
    --len;

    char *p = (char *)calloc(len + 1, sizeof(char));
    if(p == NULL)
        goto error_release;


    size_t offset = 0;
    for(int i = 0; i < n; ++i)
    {
        char *arg = va_arg(ap2, char *);
        offset += strlen(arg);

        strncat(p, arg, len);

        if(i < (n - 1))
        {
            *(p + offset++) = separator;
        }
    }

    va_end(ap);
    va_end(ap2);

    return p;


error_release:
    va_end(ap);
    va_end(ap2);

    return NULL;
}




/**
 */
char *get_filename(const char *path, char separator)
{
    const char *p_sep;
    const char *p_prev;

    for(p_sep = path; p_sep != NULL; p_sep = strchr(p_sep + 1, separator))
    {
        p_prev = p_sep;
    }

    if(p_prev == NULL)
    {
        return alloc_copy(path);
    }

    return alloc_copy(p_prev + 1);
}



/**
 */
char *get_filename_without_extensions(
        const char *path,
        char separator)
{
    char *p_ret = get_filename(path, separator);
    if(p_ret == NULL)
        return NULL;

    char *p_sep = NULL, *p_prev = NULL;
    for(p_sep = p_ret; p_sep != NULL; p_sep = strchr(p_sep + 1, '.'))
    {
        p_prev = p_sep;
    }

    if(p_prev == NULL || p_prev == p_ret)
        return p_ret;

    memset((void *)p_prev, 0, strlen(p_prev) * sizeof(char));

    char *p_realloc = (char *)realloc(p_ret, sizeof(char) * (p_prev - p_ret));

#ifdef __DEBUG__
    fprintf(stderr, "%p, %p, %p\n", p_ret, p_prev, p_realloc);
#endif

    return p_realloc;
}



static char *alloc_copy(const char *p)
{
    long len = strlen(p);

    char *p_ret = calloc(len + 1, sizeof(char));
    if(p_ret == NULL)
        return NULL;

    strncpy(p_ret, p, len);

    return p_ret;
}
