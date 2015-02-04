#ifndef NCLYR_TUI_TUI_PRINTF_H
#define NCLYR_TUI_TUI_PRINTF_H

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

#endif
