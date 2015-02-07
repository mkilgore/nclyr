
#include "common.h"

#include <string.h>

#include "printf/compiled.h"
#include "arg_int.h"
#include "debug.h"

struct printf_opt_arg_int {
    struct printf_opt opt;
    int arg;
    int width;
};

static void print_arg_int(struct printf_opt *opt, struct tui_printf_compiled *comp, WINDOW *win, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_arg_int *arg = container_of(opt, struct printf_opt_arg_int, opt);
    wprintw(win, "%d", args[arg->arg].u.int_val);
}

struct printf_opt *printf_arg_parse_int(int index, char *id_par, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_arg_int *arg = malloc(sizeof(*arg));
    memset(arg, 0, sizeof(*arg));
    arg->opt.print = print_arg_int;
    arg->opt.clear = printf_opt_free;
    arg->arg = index;
    return &arg->opt;
}


