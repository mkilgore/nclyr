#ifndef NCLYR_TUI_PRINTF_PRINTF_COLOR_H
#define NCLYR_TUI_PRINTF_PRINTF_COLOR_H

#include "compiled.h"

struct printf_opt *print_color_get(const char *id, char *params, size_t arg_count, const struct tui_printf_arg *args);

#endif
