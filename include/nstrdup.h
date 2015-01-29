#ifndef INCLUDE_NSTRDUP_H
#define INCLUDE_NSTRDUP_H

#include <string.h>

/* A replacement strdup that returns NULL when passed NULL */
static inline char *nstrdup(const char *str)
{
    if (str)
        return strdup(str);
    else
        return NULL;
}

#endif
