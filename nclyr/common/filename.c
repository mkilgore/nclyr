
#include "common.h"

#include <stdlib.h>
#include <string.h>

#include "a_sprintf.h"
#include "filename.h"
#include "debug.h"

char *filename_get(const char *s)
{
    char *home;
    char *file = NULL;
    if (s[0] != '~')
        return strdup(s);

    home = getenv("HOME");

    a_sprintf(&file, "%s%s", home, s + 1);
    return file;
}

