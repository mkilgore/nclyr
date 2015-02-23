#ifndef NCLYR_TUI_PRINTF_PRINTF_IF_H
#define NCLYR_TUI_PRINTF_PRINTF_IF_H

#include "compiler.h"

struct printf_opt *print_if_get(const char *id, char **c, char *params, size_t arg_count, const struct cons_printf_arg *args);

#endif
