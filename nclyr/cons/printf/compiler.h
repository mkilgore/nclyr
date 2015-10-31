#ifndef NCLYR_TUI_PRINTF_COMPILED_H
#define NCLYR_TUI_PRINTF_COMPILED_H

#include <stdlib.h>

#include "cons/color.h"
#include "cons/str.h"
#include "cons/printf.h"

enum printf_opt_type {
    PRINTF_OPT_CALLBACK,
    PRINTF_OPT_STRING,
};

/* One piece to print - Linked list of pieces, printed one at a time.
 *
 * To speed things up, we embed the information for the most common op,
 * printing a string, into the 'printf_opt' setup. In the future, more options
 * could be embedded into here as needed. */
struct printf_opt {
    struct printf_opt *next;
    enum printf_opt_type type;

    char *s;

    void (*print) (struct printf_opt *, struct cons_printf_compiled *, struct cons_str *, size_t arg_count, const struct cons_printf_arg *);
    void (*clear) (struct printf_opt *);
};

struct cons_printf_compiled {
    struct printf_opt *head;
    chtype attributes;
};

cons_printf_compiled *cons_printf_compile_internal(char **c, size_t arg_count, const struct cons_printf_arg *args, const char *stop_id);

void printf_opt_free(struct printf_opt *);
char *printf_get_next_param(char *params, char **id, char **val);

#define CONS_PRINTF_COMP_ATTRS(comp) ((comp)->attributes)

#endif
