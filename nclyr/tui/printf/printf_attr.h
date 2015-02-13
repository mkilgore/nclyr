#ifndef NCLYR_TUI_PRINTF_PRINTF_ATTR_H
#define NCLYR_TUI_PRINTF_PRINTF_ATTR_H

#include "compiler.h"

struct printf_opt *print_attr_get(const char *id, char **c, char *params, size_t arg_count, const struct tui_printf_arg *args);

#endif
