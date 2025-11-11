#ifndef _VALUE_DESCRIPTION_TYPE_H
#define _VALUE_DESCRIPTION_TYPE_H

#include <stdint.h>


struct value_description_t
{
    const char *name;
    uint64_t value;
    const char *description;
};


#endif
