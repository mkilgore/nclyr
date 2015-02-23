
#include "common.h"

#include <string.h>

#include "stringcasecmp.h"
#include "cons/color.h"
#include "cons/str.h"
#include "cons/printf.h"
#include "printf/compiler.h"
#include "arg_int.h"
#include "debug.h"

struct printf_opt_arg_int {
    struct printf_opt opt;
    int arg;
    int width;

    unsigned int zero_pad :1;
};

static void print_arg_int(struct printf_opt *opt, struct cons_printf_compiled *comp, struct cons_str *chstr, size_t arg_count, const struct cons_printf_arg *args)
{
    char buffer[19], *c = buffer + sizeof(buffer) - 1;
    int i, len = 0;
    struct printf_opt_arg_int *arg = container_of(opt, struct printf_opt_arg_int, opt);

    *c = '\0';

    for (i = args[arg->arg].u.int_val; i != 0; i /= 10, len++)
        *--c = '0' + (i % 10);

    if (arg->width > 0)
        for (; len <  arg->width; len++)
            *--c = (arg->zero_pad)? '0': ' ';

    cons_str_add_str(chstr, c, CONS_PRINTF_COMP_ATTRS(comp));
}

struct printf_opt *printf_arg_parse_int(int index, char *params, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_arg_int *arg = malloc(sizeof(*arg));
    char *val, *i;
    memset(arg, 0, sizeof(*arg));
    arg->opt.print = print_arg_int;
    arg->opt.clear = printf_opt_free;
    arg->arg = index;
    while ((params = printf_get_next_param(params, &i, &val)) != NULL) {
        if (strcmp(i, "w") == 0 || strcmp(i, "width") == 0)
            arg->width = strtol(val, NULL, 0);
        if (strcmp(i, "z") == 0 || strcmp(i, "zero-pad") == 0)
            arg->zero_pad = !stringcasecmp(val, "true");
    }
    return &arg->opt;
}


