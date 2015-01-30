#ifndef INCLUDE_BUILD_H
#define INCLUDE_BUILD_H

#include <stdio.h>

struct build_option {
    const char *name;
    unsigned int build :1;
};

extern struct build_option build_options[];

void build_settings_print(FILE *file);

#endif
