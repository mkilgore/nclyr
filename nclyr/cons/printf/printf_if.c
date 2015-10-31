
#include "common.h"

#include <string.h>

#include "stringcasecmp.h"
#include "cons/color.h"
#include "cons/printf.h"
#include "compiler.h"
#include "printf_if.h"
#include "debug.h"

struct arg_check {
    int arg;
    enum cons_arg_type type;
    union {
        int int_val;
        char *str_val;
        int bool_val;
        int time_val;
    } data;
};

struct printf_opt_if {
    struct printf_opt opt;

    struct arg_check check;
    cons_printf_compiled *print;
};

static void print_if(struct printf_opt *opt, struct cons_printf_compiled *comp, struct cons_str *chstr, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_if *if_stmt = container_of(opt, struct printf_opt_if, opt);
    int print = 0;

    switch (if_stmt->check.type) {
    case CONS_ARG_BOOL:
        if (args[if_stmt->check.arg].u.bool_val == if_stmt->check.data.bool_val)
            print = 1;
        break;
    case CONS_ARG_INT:
        if (args[if_stmt->check.arg].u.int_val == if_stmt->check.data.int_val)
            print = 1;
        break;
    case CONS_ARG_STRING:
        if (strcmp(args[if_stmt->check.arg].u.str_val, if_stmt->check.data.str_val) == 0)
            print = 1;
        break;
    case CONS_ARG_TIME:
        if (args[if_stmt->check.arg].u.time_val == if_stmt->check.data.time_val)
            print = 1;
        break;
    case CONS_ARG_SONG:
        if (args[if_stmt->check.arg].u.song.s)
            print = 1;
        break;
    }

    if (print) {
        struct cons_str new_chstr;
        cons_printf(if_stmt->print, &new_chstr, chstr->max_width, CONS_PRINTF_COMP_ATTRS(comp), args, arg_count);
        cons_str_add_cons_str(chstr, &new_chstr);
        comp->attributes = if_stmt->print->attributes;
        cons_str_clear(&new_chstr);
    }
}

static void printf_free(struct printf_opt *opt)
{
    struct printf_opt_if *if_stmt = container_of(opt, struct printf_opt_if, opt);

    cons_printf_compiled_free(if_stmt->print);

    if (if_stmt->check.type == CONS_ARG_STRING)
        free(if_stmt->check.data.str_val);

    free(if_stmt);
}

struct printf_opt *print_if_get(const char *id, char **c, char *params, size_t arg_count, const struct cons_printf_arg *args)
{
    int i;
    struct printf_opt_if *if_stmt = malloc(sizeof(*if_stmt));
    char *val, *p;
    memset(if_stmt, 0, sizeof(*if_stmt));
    if_stmt->opt.print = print_if;
    if_stmt->opt.clear = printf_free;
    while ((params = printf_get_next_param(params, &p, &val)) != NULL) {
        for (i = 0; i < arg_count; i++) {
            if (strcmp(p, args[i].id) == 0) {
                if_stmt->check.arg = i;
                if_stmt->check.type = args[i].type;
                switch (args[i].type) {
                case CONS_ARG_INT:
                    if_stmt->check.data.int_val = strtol(val, NULL, 0);
                    break;
                case CONS_ARG_STRING:
                    if_stmt->check.data.str_val = strdup(val);
                    break;
                case CONS_ARG_BOOL:
                    if_stmt->check.data.bool_val = (stringcasecmp(val, "true") == 0);
                    break;
                case CONS_ARG_TIME:
                    if_stmt->check.data.time_val = strtol(val, NULL, 0);
                    break;
                case CONS_ARG_SONG:
                    break;
                }
            }
        }
    }
    if_stmt->print = cons_printf_compile_internal(c, arg_count, args, "endif");
    return &if_stmt->opt;
}
