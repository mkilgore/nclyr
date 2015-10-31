
#include "common.h"

#include <string.h>

#include "cons/color.h"
#include "cons/str.h"
#include "printf/compiler.h"
#include "arg_bool.h"
#include "debug.h"

struct printf_opt_arg_bool {
    struct printf_opt opt;
    int arg;
    char *s_true, *s_false;
};

static void print_arg_bool(struct printf_opt *opt, struct cons_printf_compiled *comp, struct cons_str *chstr, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_arg_bool *bol = container_of(opt, struct printf_opt_arg_bool, opt);
    if (args[bol->arg].u.bool_val)
        cons_str_add_str(chstr, bol->s_true, CONS_PRINTF_COMP_ATTRS(comp));
    else
        cons_str_add_str(chstr, bol->s_false, CONS_PRINTF_COMP_ATTRS(comp));
}

struct printf_opt *printf_arg_parse_bool(int index, char *params, size_t arg_count, const struct cons_printf_arg *args)
{
    char *val, *p;
    struct printf_opt_arg_bool *bol = malloc(sizeof(*bol));
    memset(bol, 0, sizeof(*bol));
    bol->opt.print = print_arg_bool;
    bol->opt.clear = printf_opt_free;
    bol->arg = index;

    while ((params = printf_get_next_param(params, &p, &val)) != NULL) {
        if (strcmp(p, "t") == 0 || strcmp(p, "true") == 0)
            bol->s_true = strdup(val);
        if (strcmp(p, "f") == 0 || strcmp(p, "false") == 0)
            bol->s_false = strdup(val);
    }

    if (!bol->s_true)
        bol->s_true = strdup("true");

    if (!bol->s_false)
        bol->s_false = strdup("false");

    return &bol->opt;
}

