#ifndef NCLYR_TUI_PRINTF_PRINTF_ARG_H
#define NCLYR_TUI_PRINTF_PRINTF_ARG_H

#include "tui/printf.h"

struct printf_opt *printf_arg_get(int index, char *params, size_t arg_count, const struct tui_printf_arg *args);

#endif
