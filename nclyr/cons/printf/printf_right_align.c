
#include "common.h"

#include <string.h>
#include <stdlib.h>

#include "cons/color.h"
#include "compiler.h"
#include "printf_right_align.h"
#include "debug.h"

struct printf_opt_right_align {
    struct printf_opt opt;

    cons_printf_compiled *print;
};

static void print_right_align(struct printf_opt *opt, struct cons_printf_compiled *comp, struct cons_str *chstr, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_right_align *ra = container_of(opt, struct printf_opt_right_align, opt);
    struct cons_str new_chstr;

    cons_printf(ra->print, &new_chstr, chstr->max_width, CONS_PRINTF_COMP_ATTRS(comp), args, arg_count);

    if (chstr->max_width) {
        if (chstr->max_width - new_chstr.length >= 0)
            cons_str_add_cons_str_at(chstr, &new_chstr, chstr->max_width - new_chstr.length);
        else
            cons_str_add_cons_str_at(chstr, &new_chstr, 0);
    } else {
        cons_str_add_cons_str(chstr, &new_chstr);
    }

    cons_str_clear(&new_chstr);
}

static void print_right_align_free(struct printf_opt *opt)
{
    struct printf_opt_right_align *ra = container_of(opt, struct printf_opt_right_align, opt);
    cons_printf_compiled_free(ra->print);

    free(ra);
}

struct printf_opt *print_right_align_get(const char *id, char **c, char *params, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_right_align *ra = malloc(sizeof(*ra));
    memset(ra, 0, sizeof(*ra));
    ra->opt.print = print_right_align;
    ra->opt.clear = print_right_align_free;

    ra->print = cons_printf_compile_internal(c, arg_count, args, NULL);
    return &ra->opt;
}

