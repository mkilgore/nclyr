#ifndef INCLUDE_TUI_PRINTF_H
#define INCLUDE_TUI_PRINTF_H

#include <ncurses.h>

#include "tui_chstr.h"

enum tui_arg_type {
    TUI_ARG_INT,
    TUI_ARG_STRING,
    TUI_ARG_BOOL,
};

union tui_printf_data {
    int int_val;
    const char *str_val;
    int bool_val;
};

struct tui_printf_arg {
    const char *id;
    enum tui_arg_type type;
    union tui_printf_data u;
};

struct tui_printf_compiled;

/* We're typedef'ing this, because nobody should be attempting to access it's
 * members */
typedef struct tui_printf_compiled tui_printf_compiled;

/* Note, for compiling: The 'args' array should keep the same order, as the
 * compiled printing may depend on using indexes into it. */
tui_printf_compiled *tui_printf_compile(const char *format, size_t arg_count, const struct tui_printf_arg *args);

void tui_printf(struct chstr *, chtype attrs, int max_width, tui_printf_compiled *, size_t arg_count, const struct tui_printf_arg *args);

void tui_printf_compile_free(tui_printf_compiled *);

chtype tui_get_chtype_from_window(WINDOW *);


#endif
