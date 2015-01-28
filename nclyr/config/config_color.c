#include "common.h"

#include "stringcasecmp.h"
#include "debug.h"

static const char *color_names[] = { "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white" };

int config_color_get(const char *name)
{
    int i;
    for (i = 0; i < sizeof(color_names)/sizeof(*color_names); i++)
        if (stringcasecmp(color_names[i], name) == 0)
            return i;
    return -1;
}

const char *config_color_name(int color)
{
    if (color == -1)
        return "default";
    else
        return color_names[color];
}

