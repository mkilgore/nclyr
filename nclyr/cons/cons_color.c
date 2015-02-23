
#include "common.h"

#include "stringcasecmp.h"
#include "cons/color.h"
#include "debug.h"

static const char *color_names[] = { "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white" };

enum cons_color cons_color_get(const char *name)
{
    int i;
    for (i = 0; i < sizeof(color_names)/sizeof(*color_names); i++)
        if (stringcasecmp(color_names[i], name) == 0)
            return i;
    return CONS_COLOR_DEFAULT;
}

const char *cons_color_name(enum cons_color color)
{
    return color_names[color];
}

