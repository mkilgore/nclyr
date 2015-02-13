#ifndef NCLYR_TUI_PRINTF_COMPILED_H
#define NCLYR_TUI_PRINTF_COMPILED_H

#include <stdlib.h>
#include <ncurses.h>

#include "cons_color.h"
#include "tui_printf.h"
#include "tui_chstr.h"

/* One piece to print - Linked list of pieces, printed one at a time */
struct printf_opt {
    struct printf_opt *next;
    void (*print) (struct printf_opt *, struct tui_printf_compiled *, struct chstr *, size_t arg_count, const struct tui_printf_arg *);
    void (*clear) (struct printf_opt *);
};

struct tui_printf_compiled {
    struct printf_opt *head;
    struct cons_color_pair colors;
    chtype attributes;
};

tui_printf_compiled *tui_printf_compile_internal(char **c, size_t arg_count, const struct tui_printf_arg *args, const char *stop_id);

void printf_opt_free(struct printf_opt *);
char *printf_get_next_param(char *params, char **id, char **val);

#endif
