
#include <stdio.h>

#include "config.h"

void c_begin(void)
{
    printf(" /*\n"
           "  * File generated by 'config'\n"
           "  *\n"
           "  * ### DO NOT EDIT THIS FILE DIRECTLY ###\n"
           "  * ###     IT WILL BE OVERWRITTEN     ###\n"
           "  */\n"
           "#ifndef INCLUDE_GEN_CONFIG_H\n"
           "#define INCLUDE_GEN_CONFIG_H\n"
           "\n");
}

void c_end(void)
{
    printf("\n"
           "#endif\n");
}

void c_write_var(enum output_type type, const char *prefix, const char *var, const char *dat)
{
    switch (type) {
    case OUTPUT_YES:
        printf("#define CONFIG_%s%s 1\n", prefix? prefix: "", var);
        break;
    case OUTPUT_NO:
        printf("#define CONFIG_%s%s 0\n", prefix? prefix: "", var);
        break;
    case OUTPUT_QUOTE_STRING:
        printf("#define CONFIG_%s%s \"%s\"\n", prefix? prefix: "", var, dat);
        break;
    case OUTPUT_NOQUOTE_STRING:
        printf("#define CONFIG_%s%s %s\n", prefix? prefix: "", var, dat);
        break;
    }
}

struct config_output c_output = {
    .start = c_begin,
    .end = c_end,
    .write_var = c_write_var,
};

