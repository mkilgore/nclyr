
#include "common.h"

#include <string.h>

#include "cons/color.h"
#include "cons/str.h"
#include "cons/printf.h"
#include "printf/compiler.h"
#include "arg_string.h"
#include "debug.h"

struct printf_opt_arg_string {
    struct printf_opt opt;
    int arg;
};

static void print_arg_string(struct printf_opt *opt, struct cons_printf_compiled *comp, struct cons_str *chstr, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_arg_string *arg = container_of(opt, struct printf_opt_arg_string, opt);
    cons_str_add_str(chstr, args[arg->arg].u.str_val, comp->attributes | COLOR_PAIR(cons_color_pair_to_num(&comp->colors)));
}

struct printf_opt *printf_arg_parse_string(int index, char *id_par, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_arg_string *arg = malloc(sizeof(*arg));
    memset(arg, 0, sizeof(*arg));
    arg->opt.print = print_arg_string;
    arg->opt.clear = printf_opt_free;
    arg->arg = index;
    return &arg->opt;
}



