#ifndef INCLUDE_TUI_PRINTF_H
#define INCLUDE_TUI_PRINTF_H

#include <ncurses.h>

enum tui_arg_type {
    TUI_ARG_INT,
    TUI_ARG_STRING,
};

struct tui_printf_arg {
    const char *id;
    enum tui_arg_type type;
    union {
        int int_val;
        const char *str_val;
    } u;
};

void tui_printf(WINDOW *win, const char *format, size_t arg_count, const struct tui_printf_arg *args);

struct tui_printf_compiled;

/* We're typedef'ing this, because nobody should be attempting to access it's
 * members */
typedef struct tui_printf_compiled tui_printf_compiled;

/* Note, for compiling: The 'args' array should keep the same order, as the
 * compiled printing may depend on using indexes into it. */
tui_printf_compiled *tui_printf_compile(const char *format, size_t arg_count, const struct tui_printf_arg *args);

void tui_printf_comp(WINDOW *, tui_printf_compiled *, size_t arg_count, const struct tui_printf_arg *args);

void tui_printf_compile_free(tui_printf_compiled *);


#endif
