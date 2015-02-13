
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "cons_color.h"
#include "tui_internal.h"
#include "tui_color.h"
#include "tui_printf.h"
#include "tui_chstr.h"
#include "printf_string.h"
#include "printf_attr.h"
#include "printf_arg.h"
#include "printf_color.h"
#include "printf_if.h"
#include "compiler.h"
#include "debug.h"

struct printf_cmd {
    const char *id;
    struct printf_opt *(*get) (const char *id, char **c, char *params, size_t arg_count, const struct tui_printf_arg *args);
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
    { .id = "if",
        .get = print_if_get },
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


static void handle_id(struct printf_opt ***opt_next, char *id,  char **c, size_t arg_count, const struct tui_printf_arg *args)
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
            **opt_next = cmds[i].get(id, c, id_par, arg_count, args);
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

tui_printf_compiled *tui_printf_compile_internal(char **c, size_t arg_count, const struct tui_printf_arg *args, const char *stop_id)
{
    struct tui_printf_compiled *comp = malloc(sizeof(*comp));
    struct printf_opt **opt_next = &comp->head;
    char *c1;

    while ((c1 = strchr(*c, '$')) != NULL) {
        struct printf_opt *opt;

        if (c1 - *c > 0) {
            opt = print_string_getn(*c, c1 - *c);
            *opt_next = opt;
            opt_next = &opt->next;
        }
        *c = c1 + 1;

        if (**c == '$') {
            opt = print_string_get("$");
            *opt_next = opt;
            opt_next = &opt->next;
            (*c)++;
            continue;
        }

        if (**c == '{') {
            char *id, *end;
            (*c)++;
            end = strchr(*c, '}');
            if (end == NULL)
                goto cleanup;

            id = *c;
            *c = end + 1;
            *end = '\0';

            if (stop_id)
                if (strcmp(id, stop_id) == 0)
                   goto cleanup;

            handle_id(&opt_next, id, c, arg_count, args);
        }
    }

cleanup:
    return comp;
}

tui_printf_compiled *tui_printf_compile(const char *format, size_t arg_count, const struct tui_printf_arg *args)
{
    tui_printf_compiled *comp;
    char *c_orig = strdup(format);
    char *c = c_orig;

    comp = tui_printf_compile_internal(&c, arg_count, args, NULL);

    free(c_orig);
    return comp;
}

static chtype attr_t_to_chtype(attr_t attrs)
{
    chtype attributes = 0;
    if (attrs & WA_BOLD)
        attributes |= A_BOLD;
    if (attrs & WA_REVERSE)
        attributes |= A_REVERSE;
    if (attrs & WA_DIM)
        attributes |= A_DIM;
    if (attrs & WA_BLINK)
        attributes |= A_BLINK;
    if (attrs & WA_UNDERLINE)
        attributes |= A_UNDERLINE;
    return attributes;
}

/* The WINDOW * is used to get the current attributes */
void tui_printf(struct chstr *chstr, chtype attrs, int max_width, tui_printf_compiled *print, size_t arg_count, const struct tui_printf_arg *args)
{
    struct tui_printf_compiled *comp = print;
    struct printf_opt *cur;

    chstr_init(chstr);

    if (max_width)
        chstr_setwidth(chstr, max_width);

    comp->attributes = attrs & A_ATTRIBUTES;
    tui_color_pair_fb(PAIR_NUMBER(attrs), &comp->colors);

    for (cur = comp->head; cur; cur = cur->next)
        (cur->print) (cur, comp, chstr, arg_count, args);

    return ;
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

chtype tui_get_chtype_from_window(WINDOW *win)
{
    chtype ret = 0;
    int col_pair;
    attr_t attrs;

    wattr_get(win, &attrs, &col_pair, NULL);

    ret |= COLOR_PAIR(col_pair);
    ret |= attr_t_to_chtype(attrs);

    return ret;
}

