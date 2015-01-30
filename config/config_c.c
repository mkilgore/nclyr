
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

void c_write_var(const char *var, int set)
{
    if (set)
        printf("#define %s 1\n", var);
    else
        printf("#define %s 0\n", var);
}

struct config_output c_output = {
    .start = c_begin,
    .end = c_end,
    .write_var = c_write_var,
};

