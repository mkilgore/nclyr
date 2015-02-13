
#include "common.h"

#include <string.h>

#include "stringcasecmp.h"
#include "tui_color.h"
#include "printf/compiler.h"
#include "arg_int.h"
#include "debug.h"

struct printf_opt_arg_int {
    struct printf_opt opt;
    int arg;
    int width;

    unsigned int zero_pad :1;
};

static void print_arg_int(struct printf_opt *opt, struct tui_printf_compiled *comp, struct chstr *chstr, size_t arg_count, const struct tui_printf_arg *args)
{
    char buffer[19], *c = buffer + sizeof(buffer);
    int i, len = 0;
    struct printf_opt_arg_int *arg = container_of(opt, struct printf_opt_arg_int, opt);

    *c = '\0';

    for (i = args[arg->arg].u.int_val; i != 0; i /= 10, len++)
        *--c = '0' + (i % 10);

    if (arg->width > 0)
        for (; len <  arg->width; len++)
            *--c = (arg->zero_pad)? '0': ' ';

    chstr_addstr(chstr, c, comp->attributes | COLOR_PAIR(tui_color_pair_get(&comp->colors)));
}

struct printf_opt *printf_arg_parse_int(int index, char *params, size_t arg_count, const struct tui_printf_arg *args)
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
            arg->zero_pad = stringcasecmp(val, "true")? 1 : 0;
    }
    return &arg->opt;
}


