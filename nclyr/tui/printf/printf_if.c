
#include "common.h"

#include <string.h>
#include <ncurses.h>

#include "stringcasecmp.h"
#include "tui_color.h"
#include "compiler.h"
#include "printf_if.h"
#include "debug.h"

struct arg_check {
    int arg;
    enum tui_arg_type type;
    union {
        int int_val;
        char *str_val;
        int bool_val;
    } data;
};

struct printf_opt_if {
    struct printf_opt opt;

    struct arg_check check;
    tui_printf_compiled *print;
};

static void print_if(struct printf_opt *opt, struct tui_printf_compiled *comp, struct chstr *chstr, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_if *if_stmt = container_of(opt, struct printf_opt_if, opt);

    switch (if_stmt->check.type) {
    case TUI_ARG_BOOL:
        if (args[if_stmt->check.arg].u.bool_val == if_stmt->check.data.bool_val) {
            struct chstr new_chstr;
            tui_printf(&new_chstr, comp->attributes | COLOR_PAIR(tui_color_pair_get(&comp->colors)), chstr->max_width, if_stmt->print, arg_count, args);
            chstr_addchstr(chstr, &new_chstr);
            comp->attributes = if_stmt->print->attributes;
            comp->colors = if_stmt->print->colors;
            chstr_clear(&new_chstr);
        }
        break;
    }
}

static void printf_free(struct printf_opt *opt)
{
    struct printf_opt_if *if_stmt = container_of(opt, struct printf_opt_if, opt);

    tui_printf_compile_free(if_stmt->print);

    if (if_stmt->check.type == TUI_ARG_STRING)
        free(if_stmt->check.data.str_val);

    free(if_stmt);
}

struct printf_opt *print_if_get(const char *id, char **c, char *params, size_t arg_count, const struct tui_printf_arg *args)
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
                case TUI_ARG_INT:
                    if_stmt->check.data.int_val = strtol(val, NULL, 0);
                    break;
                case TUI_ARG_STRING:
                    if_stmt->check.data.str_val = strdup(val);
                    break;
                case TUI_ARG_BOOL:
                    if_stmt->check.data.bool_val = (stringcasecmp(val, "true") == 0);
                    break;
                }
            }
        }
    }
    if_stmt->print = tui_printf_compile_internal(c, arg_count, args, "endif");
    return &if_stmt->opt;
}
