
#include "common.h"

#include <string.h>
#include <stdlib.h>

#include "tui_color.h"
#include "compiler.h"
#include "printf_right_align.h"
#include "debug.h"

struct printf_opt_right_align {
    struct printf_opt opt;

    tui_printf_compiled *print;
};

static void print_right_align(struct printf_opt *opt, struct tui_printf_compiled *comp, struct chstr *chstr, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_right_align *ra = container_of(opt, struct printf_opt_right_align, opt);
    struct chstr new_chstr;

    tui_printf(ra->print, &new_chstr, chstr->max_width, TUI_PRINTF_COMP_ATTRS(comp), args, arg_count);

    if (chstr->max_width)
        chstr_addchstr_at(chstr, &new_chstr, chstr->max_width - new_chstr.length);
    else
        chstr_addchstr(chstr, &new_chstr);

    chstr_clear(&new_chstr);
}

static void print_right_align_free(struct printf_opt *opt)
{
    struct printf_opt_right_align *ra = container_of(opt, struct printf_opt_right_align, opt);
    tui_printf_compile_free(ra->print);

    free(ra);
}

struct printf_opt *print_right_align_get(const char *id, char **c, char *params, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_right_align *ra = malloc(sizeof(*ra));
    memset(ra, 0, sizeof(*ra));
    ra->opt.print = print_right_align;
    ra->opt.clear = print_right_align_free;

    ra->print = tui_printf_compile_internal(c, arg_count, args, NULL);
    return &ra->opt;
}

