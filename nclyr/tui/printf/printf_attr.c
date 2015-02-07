
#include "common.h"

#include <string.h>

#include "compiled.h"
#include "printf_attr.h"
#include "debug.h"

static void print_bold(struct printf_opt *opt, struct tui_printf_compiled *comp, WINDOW *win, size_t arg_count, const struct tui_printf_arg *args)
{
    comp->attrs ^= WA_BOLD;
    if (comp->attrs & WA_BOLD)
        wattron(win, A_BOLD);
    else
        wattroff(win, A_BOLD);
}

static void print_reverse(struct printf_opt *opt, struct tui_printf_compiled *comp, WINDOW *win, size_t arg_count, const struct tui_printf_arg *args)
{
    comp->attrs ^= WA_REVERSE;
    if (comp->attrs & WA_REVERSE)
        wattron(win, A_REVERSE);
    else
        wattroff(win, A_REVERSE);
}

static void print_dim(struct printf_opt *opt, struct tui_printf_compiled *comp, WINDOW *win, size_t arg_count, const struct tui_printf_arg *args)
{
    comp->attrs ^= WA_DIM;
    if (comp->attrs & WA_DIM)
        wattron(win, A_DIM);
    else
        wattroff(win, A_DIM);
}

static void print_blink(struct printf_opt *opt, struct tui_printf_compiled *comp, WINDOW *win, size_t arg_count, const struct tui_printf_arg *args)
{
    comp->attrs ^= WA_BLINK;
    if (comp->attrs & WA_BLINK)
        wattron(win, A_BLINK);
    else
        wattroff(win, A_BLINK);
}

static void print_underline(struct printf_opt *opt, struct tui_printf_compiled *comp, WINDOW *win, size_t arg_count, const struct tui_printf_arg *args)
{
    comp->attrs ^= WA_UNDERLINE;
    if (comp->attrs & WA_UNDERLINE)
        wattron(win, A_UNDERLINE);
    else
        wattroff(win, A_UNDERLINE);
}

static void print_free(struct printf_opt *opt)
{
    free(opt);
}

struct printf_opt *print_attr_get(const char *id, char *params, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt *opt = malloc(sizeof(*opt));
    memset(opt, 0, sizeof(*opt));
    if (strcmp(id, "bold") == 0)
        opt->print = print_bold;
    else if (strcmp(id, "reverse") == 0)
        opt->print = print_reverse;
    else if (strcmp(id, "dim") == 0)
        opt->print = print_dim;
    else if (strcmp(id, "blink") == 0)
        opt->print = print_blink;
    else if (strcmp(id, "underline") == 0)
        opt->print = print_underline;

    opt->clear = print_free;

    return opt;
}

