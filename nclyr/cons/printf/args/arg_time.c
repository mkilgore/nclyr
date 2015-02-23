
#include "common.h"

#include <string.h>

#include "stringcasecmp.h"
#include "cons/color.h"
#include "cons/str.h"
#include "printf/compiler.h"
#include "arg_time.h"
#include "debug.h"

struct printf_opt_arg_time {
    struct printf_opt opt;
    int arg;

    unsigned int pad     :1;
    unsigned int seconds :1;
    unsigned int minutes :1;

    char split_c;
};

static void print_arg_time(struct printf_opt *opt, struct cons_printf_compiled *comp, struct cons_str *chstr, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_arg_time *tim = container_of(opt, struct printf_opt_arg_time, opt);
    int t = args[tim->arg].u.time_val;
    char num[3] = { 0 }, *c;

    if (tim->minutes) {
        c = num + sizeof(num) - 1;
        *c = '\0';
        *--c = '0' + (t / 60) % 10;
        if (tim->pad || (t / 60 / 10) != 0)
            *--c = '0' + (t / 60 / 10) % 10;
        cons_str_add_str(chstr, c, CONS_PRINTF_COMP_ATTRS(comp));

        if (tim->seconds)
            cons_str_add_ch(chstr, make_cons_char(tim->split_c) | CONS_PRINTF_COMP_ATTRS(comp));
    }

    if (tim->seconds) {
        c = num + sizeof(num) - 1;
        *c = '\0';
        *--c = '0' + (t % 60) % 10;
        if (tim->pad || (t % 60 / 10) != 0)
            *--c = '0' + (t % 60 / 10) % 10;
        cons_str_add_str(chstr, c, CONS_PRINTF_COMP_ATTRS(comp));
    }
}

struct printf_opt *printf_arg_parse_time(int index, char *params, size_t arg_count, const struct cons_printf_arg *args)
{
    char *val, *p;
    struct printf_opt_arg_time *tim = malloc(sizeof(*tim));
    memset(tim, 0, sizeof(*tim));
    tim->opt.print = print_arg_time;
    tim->opt.clear = printf_opt_free;
    tim->arg = index;
    tim->split_c = ':';
    while ((params = printf_get_next_param(params, &p, &val)) != NULL) {
        if (strcmp(p, "p") == 0 || strcmp(p, "pad") == 0)
            tim->pad = (stringcasecmp(val, "true") == 0);
        if (strcmp(p, "s") == 0 || strcmp(p, "seconds") == 0)
            tim->seconds = (stringcasecmp(val, "true") == 0);
        if (strcmp(p, "m") == 0 || strcmp(p, "minutes") == 0)
            tim->minutes = (stringcasecmp(val, "true") == 0);
        if (strcmp(p, "c") == 0 || strcmp(p, "split-char") == 0)
            tim->split_c = *val;
    }
    return &tim->opt;
}

