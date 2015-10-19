#ifndef INCLUDE_CONS_PRINTF_H
#define INCLUDE_CONS_PRINTF_H

#include <stdlib.h>

#include "color.h"
#include "str.h"

struct cons_printf_compiled;
typedef struct cons_printf_compiled cons_printf_compiled;

enum cons_arg_type {
    CONS_ARG_INT,
    CONS_ARG_STRING,
    CONS_ARG_BOOL,
    CONS_ARG_TIME,
    CONS_ARG_SONG,
};

union cons_printf_data {
    int int_val;
    const char *str_val;
    int bool_val;
    int time_val;
    struct {
        struct song_info *s;
        cons_printf_compiled *triple;
        cons_printf_compiled *filename;
    } song;
};

struct cons_printf_arg {
    const char *id;
    enum cons_arg_type type;
    union cons_printf_data u;
};

cons_printf_compiled *cons_printf_compile(const char *format, size_t arg_count, const struct cons_printf_arg *args);
cons_printf_compiled *cons_printf_compile_song(const char *format);

void cons_printf(cons_printf_compiled *, struct cons_str *, int max_width, cons_attr attrs, const struct cons_printf_arg *args, size_t arg_count);

void cons_printf_compiled_free(cons_printf_compiled *);

#endif
