
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "cons_color.h"
#include "tui_color.h"
#include "tui/printf.h"
#include "printf_string.h"
#include "printf_attr.h"
#include "printf_arg.h"
#include "printf_color.h"
#include "compiled.h"
#include "debug.h"

struct printf_cmd {
    const char *id;
    struct printf_opt *(*get) (const char *id, char *params, size_t arg_count, const struct tui_printf_arg *args);
};

static struct printf_cmd cmds[] = {
    { .id = "reverse",
        .get = print_attr_get },
    { .id = "bold",
        .get = print_attr_get },
    { .id = "dim",
        .get = print_attr_get },
    { .id = "underline",
        .get = print_attr_get },
    { .id = "blink",
        .get = print_attr_get },
    { .id = "color",
        .get = print_color_get },
};

char *printf_get_next_param(char *params, char **id, char **val)
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


static void handle_id(struct printf_opt ***opt_next, char *id, size_t arg_count, const struct tui_printf_arg *args)
{
    char *colon, *id_par = NULL;
    int i;

    colon = strchr(id, ';');
    if (colon) {
        *colon = '\0';
        id_par = colon + 1;
    }

    for (i = 0; i < ARRAY_SIZE(cmds); i++) {
        if (strcmp(id, cmds[i].id) == 0) {
            **opt_next = cmds[i].get(id, id_par, arg_count, args);
            *opt_next = &((**opt_next)->next);
            return ;
        }
    }

    for (i = 0; i < arg_count; i++) {
        if (strcmp(id, args[i].id) == 0) {
            **opt_next = printf_arg_get(i, id_par, arg_count, args);
            *opt_next = &((**opt_next)->next);
            return ;
        }
    }
}

tui_printf_compiled *tui_printf_compile(const char *format, size_t arg_count, const struct tui_printf_arg *args)
{
    struct tui_printf_compiled *comp = malloc(sizeof(*comp));
    struct printf_opt **opt_next = &comp->head;
    char *c_orig = strdup(format);
    char *c1, *c;

    c = c_orig;

    while ((c1 = strchr(c, '$')) != NULL) {
        struct printf_opt *opt;

        if (c1 - c > 0) {
            opt = print_string_getn(c, c1 - c);
            *opt_next = opt;
            opt_next = &opt->next;
        }
        c = c1 + 1;

        if (*c == '$') {
            opt = print_string_get("$");
            *opt_next = opt;
            opt_next = &opt->next;
            c++;
            continue;
        }

        if (*c == '{') {
            char *id, *end;
            c++;
            end = strchr(c, '}');
            if (end == NULL)
                goto cleanup;

            id = c;
            c = end + 1;
            *end = '\0';

            handle_id(&opt_next, id, arg_count, args);
        }
    }

cleanup:
    free(c_orig);
    return comp;
}

void tui_printf_comp(WINDOW *win, tui_printf_compiled *print, size_t arg_count, const struct tui_printf_arg *args)
{
    int col_pair;
    attr_t attr_old;
    struct tui_printf_compiled *comp = print;
    struct printf_opt *cur;

    wattr_get(win, &attr_old, &col_pair, NULL);
    comp->attrs = attr_old;
    tui_color_pair_fb(col_pair, &comp->cur_color);

    for (cur = comp->head; cur; cur = cur->next)
        (cur->print) (cur, comp, win, arg_count, args);


    wattr_set(win, attr_old, col_pair, NULL);
}

void tui_printf_compile_free(tui_printf_compiled *print)
{
    struct tui_printf_compiled *comp = print;
    struct printf_opt *cur, *tmp;

    for (cur = comp->head; cur; cur = tmp) {
        tmp = cur->next;
        (cur->clear) (cur);
    }

    free(comp);
}

void printf_opt_free(struct printf_opt *opt)
{
    free(opt);
}

