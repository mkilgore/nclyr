#ifndef NCLYR_TUI_PRINTF_ARGS_ARG_STRING_H
#define NCLYR_TUI_PRINTF_ARGS_ARG_STRING_H

#include "printf/compiler.h"

struct printf_opt *printf_arg_parse_string(int index, char *id_par, size_t arg_count, const struct tui_printf_arg *args);

#endif
