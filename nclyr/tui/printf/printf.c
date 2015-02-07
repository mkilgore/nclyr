
#include "common.h"

#include <string.h>
#include <ncurses.h>

#include "cons_color.h"
#include "tui_color.h"
#include "tui/printf.h"
#include "debug.h"

struct attr_flag {
    const char *name;
    attr_t wattr;
    unsigned int attr;
};

struct format_print_params {
    WINDOW *win;
    struct cons_color_pair cur_color;
    attr_t attrs;
    char *c;
    size_t arg_count;
    const struct tui_printf_arg *args;

    struct attr_flag flags[5];
};

static char *get_next_param(char *params, char **id, char **val)
{
    char *sep;

    if (!params)
        return NULL;

    sep = strchr(params, ':');
    if (!sep) {
        params = NULL;
        *id = NULL;
        *val = NULL;
        return params;
    }

    *id = params;
    *sep = '\0';
    *val = sep + 1;
    sep = strchr(*val, ';');
    if (!sep) {
        params = *val - 1; /* This always points to a '\0' */
    } else {
        *sep = '\0';
        params = sep + 1;
    }

    return params;
}

static void print_int(WINDOW *win, const struct tui_printf_arg *arg, char *params)
{
    char *id, *val;
    int width = 0;
    while ((params = get_next_param(params, &id, &val)) != NULL) {
        if (strcmp(id, "width") == 0 || strcmp(id, "w") == 0)
            width = strtol(val, NULL, 0);
    }
    wprintw(win, "%*d", width, arg->u.int_val);
}

static void print_string(WINDOW *win, const struct tui_printf_arg *arg, char *params)
{
    char *id, *val;
    int width = 0, max = -1;
    while ((params = get_next_param(params, &id, &val)) != NULL) {
        if (strcmp(id, "width") == 0 || strcmp(id, "w") == 0)
            width = strtol(val, NULL, 0);
        if (strcmp(id, "max") == 0)
            max = strtol(val, NULL, 0);
    }
    if (max < 0)
        wprintw(win, "%*s", width, arg->u.str_val);
    else
        wprintw(win, "%*.*s", width, max, arg->u.str_val);
}

static void print_arg(WINDOW *win, const struct tui_printf_arg *arg, char *params)
{
    void (*print[])(WINDOW *, const struct tui_printf_arg *, char *) = {
        [TUI_ARG_INT] = print_int,
        [TUI_ARG_STRING] = print_string,
    };

    (print[arg->type]) (win, arg, params);
}

static void handle_color(WINDOW *win, struct format_print_params *params, char *c)
{
    char *id, *val;

    tui_color_unset(params->win, params->cur_color);
    while ((c = get_next_param(c, &id, &val)) != NULL) {
        if (strcmp(id, "f") == 0 || strcmp(id, "foreground") == 0)
            params->cur_color.f = cons_color_get(val);
        if (strcmp(id, "b") == 0 || strcmp(id, "background") == 0)
            params->cur_color.b = cons_color_get(val);
    }
    tui_color_set(params->win, params->cur_color);
}

static void handle_modifier(struct format_print_params *params, char *id)
{
    char *colon, *id_par = NULL;
    int i;

    colon = strchr(id, ';');
    if (colon) {
        *colon = '\0';
        id_par = colon + 1;
    }

    if (strcmp(id, "color") == 0) {
        handle_color(params->win, params, id_par);
        return ;
    }

    for (i = 0; i < ARRAY_SIZE(params->flags); i++) {
        if (strcmp(id, params->flags[i].name) == 0) {
            params->attrs ^= params->flags[i].wattr;
            if (params->attrs & params->flags[i].wattr)
                wattron(params->win, params->flags[i].attr);
            else
                wattroff(params->win, params->flags[i].attr);
            return ;
        }
    }

    for (i = 0; i < params->arg_count; i++)
        if (strcmp(id, params->args[i].id) == 0)
            print_arg(params->win, params->args + i, id_par);
}

void tui_printf(WINDOW *windo, const char *format, size_t arg_count, const struct tui_printf_arg *args)
{
    struct format_print_params params = {
        .win = windo,
        .c = NULL,
        .arg_count = arg_count,
        .args = args,
        .cur_color = { 0, 0 },
        .attrs = 0,
        .flags = {
            { "reverse", WA_REVERSE, A_REVERSE },
            { "bold", WA_BOLD, A_BOLD },
            { "underline", WA_UNDERLINE, A_UNDERLINE },
            { "blink", WA_BLINK, A_BLINK },
            { "dim", WA_DIM, A_DIM },
        },
    };
    char *c_orig = strdup(format);
    attr_t attr_old;
    int pair_old;
    char *c1;

    /* Grab starting attrs - We restore these at the end of a print */
    wattr_get(params.win, &attr_old, &pair_old, NULL);

    params.attrs = attr_old;
    tui_color_pair_fb(pair_old, &params.cur_color);

    params.c = c_orig;

    while ((c1 = strchr(params.c, '$')) != NULL) {
        wprintw(params.win, "%.*s", c1 - params.c, params.c);
        params.c = c1 + 1;

        if (*params.c == '$') {
            wprintw(params.win, "$");
            params.c++;
            continue;
        }

        if (*params.c == '{') {
            char *id;
            char *end;
            params.c++;
            end = strchr(params.c, '}');

            if (end == NULL)
                goto cleanup;

            id = params.c;

            params.c = end + 1;
            *end = '\0';

            handle_modifier(&params, id);
        }
    }

    wprintw(params.win, "%s", params.c);

cleanup:
    wattr_set(params.win, attr_old, pair_old, NULL);
    free(c_orig);
}

