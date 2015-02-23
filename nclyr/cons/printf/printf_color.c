
#include "common.h"

#include <string.h>
#include <ncurses.h>

#include "cons/color.h"
#include "cons/printf.h"
#include "compiler.h"
#include "printf_color.h"
#include "debug.h"

struct printf_opt_color {
    struct printf_opt opt;
    struct cons_color_pair new_col;
    unsigned int have_f :1;
    unsigned int have_b :1;
};

static void print_color(struct printf_opt *opt, struct cons_printf_compiled *comp, struct cons_str *chstr, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_color *color = container_of(opt, struct printf_opt_color, opt);
    if (color->have_f)
        comp->colors.f = color->new_col.f;

    if (color->have_b)
        comp->colors.b = color->new_col.b;
}

static void print_free(struct printf_opt *opt)
{
    free(opt);
}

struct printf_opt *print_color_get(const char *id, char **c, char *params, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt_color *color = malloc(sizeof(*color));
    char *val, *i;
    memset(color, 0, sizeof(*color));
    color->opt.print = print_color;
    color->opt.clear = print_free;
    color->new_col = (struct cons_color_pair){ CONS_COLOR_DEFAULT, CONS_COLOR_DEFAULT };
    while ((params = printf_get_next_param(params, &i, &val)) != NULL) {
        if (strcmp(i, "f") == 0 || strcmp(i, "foreground") == 0) {
            color->new_col.f = cons_color_get(val);
            color->have_f = 1;
        }
        if (strcmp(i, "b") == 0 || strcmp(i, "background") == 0) {
            color->new_col.b = cons_color_get(val);
            color->have_b = 1;
        }
    }
    return &color->opt;
}
