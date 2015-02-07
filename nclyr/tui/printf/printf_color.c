
#include "common.h"

#include <string.h>
#include <ncurses.h>

#include "cons_color.h"
#include "tui_color.h"
#include "compiled.h"
#include "printf_color.h"
#include "debug.h"

struct printf_opt_color {
    struct printf_opt opt;
    struct cons_color_pair new_col;
};

static void print_color(struct printf_opt *opt, struct tui_printf_compiled *comp, WINDOW *win, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_color *color = container_of(opt, struct printf_opt_color, opt);
    tui_color_unset(win, comp->cur_color);

    if (color->new_col.f != -1)
        comp->cur_color.f = color->new_col.f;

    if (color->new_col.b != -1)
        comp->cur_color.b = color->new_col.b;

    tui_color_set(win, comp->cur_color);
}

static void print_free(struct printf_opt *opt)
{
    free(opt);
}

struct printf_opt *print_color_get(const char *id, char *params, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt_color *color = malloc(sizeof(*color));
    char *val, *i;
    memset(color, 0, sizeof(*color));
    color->opt.print = print_color;
    color->opt.clear = print_free;
    color->new_col = (struct cons_color_pair){ -1, -1 };
    while ((params = printf_get_next_param(params, &i, &val)) != NULL) {
        if (strcmp(i, "f") == 0 || strcmp(i, "foreground") == 0)
            color->new_col.f = cons_color_get(val);
        if (strcmp(i, "b") == 0 || strcmp(i, "background") == 0)
            color->new_col.b = cons_color_get(val);
    }
    return &color->opt;
}
