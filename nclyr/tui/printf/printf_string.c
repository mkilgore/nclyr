
#include "common.h"

#include <string.h>

#include "tui/printf.h"
#include "compiled.h"
#include "debug.h"

struct printf_opt_str {
    struct printf_opt opt;
    char *s;
};

static void print_string_print(struct printf_opt *opt, struct tui_printf_compiled *comp, WINDOW *win, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_str *str = container_of(opt, struct printf_opt_str, opt);
    wprintw(win, "%s", str->s);
}

static void print_string_clear(struct printf_opt *opt)
{
    struct printf_opt_str *str = container_of(opt, struct printf_opt_str, opt);
    free(str->s);
    free(str);
}

static struct printf_opt_str *printf_opt_str_new(void)
{
    struct printf_opt_str *po = malloc(sizeof(*po));
    memset(po, 0, sizeof(*po));
    po->opt.print = print_string_print;
    po->opt.clear = print_string_clear;
    return po;
}

struct printf_opt *print_string_get(const char *s)
{
    struct printf_opt_str *po = printf_opt_str_new();
    po->s = strdup(s);
    return &po->opt;
}

struct printf_opt *print_string_getn(const char *s, size_t len)
{
    struct printf_opt_str *po = printf_opt_str_new();
    po->s = strndup(s, len);
    return &po->opt;
}

