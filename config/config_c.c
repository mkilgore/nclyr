
#include <stdio.h>

#include "config.h"

void c_begin(void)
{
    printf("#ifndef INCLUDE_GEN_CONFIG_H\n"
           "#define INCLUDE_GEN_CONFIG_H\n");
}

void c_end(void)
{
    printf("#endif\n");
}

void c_write_var(const char *var, int set, const char *dat)
{
    if (set == 1)
        printf("#define %s 1\n", var);
    else if (set == 0)
        printf("#define %s 0\n", var);
    else if (set == 2)
        printf("#define %s %s\n", var, dat);
}

struct config_output c_output = {
    .start = c_begin,
    .end = c_end,
    .write_var = c_write_var,
};

