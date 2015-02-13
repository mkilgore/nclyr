
#include "common.h"

#include <string.h>

#include "tui_chstr.h"
#include "tui_color.h"
#include "printf/compiler.h"
#include "arg_string.h"
#include "debug.h"

struct printf_opt_arg_string {
    struct printf_opt opt;
    int arg;
};

static void print_arg_string(struct printf_opt *opt, struct tui_printf_compiled *comp, struct chstr *chstr, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_arg_string *arg = container_of(opt, struct printf_opt_arg_string, opt);
    chstr_addstr(chstr, args[arg->arg].u.str_val, comp->attributes | COLOR_PAIR(tui_color_pair_get(&comp->colors)));
}

struct printf_opt *printf_arg_parse_string(int index, char *id_par, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_arg_string *arg = malloc(sizeof(*arg));
    memset(arg, 0, sizeof(*arg));
    arg->opt.print = print_arg_string;
    arg->opt.clear = printf_opt_free;
    arg->arg = index;
    return &arg->opt;
}



