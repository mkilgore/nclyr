#ifndef NCLYR_TUI_PRINTF_PRINTF_STRING_H
#define NCLYR_TUI_PRINTF_PRINTF_STRING_H

#include <stdlib.h>

#include "compiler.h"

struct printf_opt *print_string_get(const char *s);
struct printf_opt *print_string_getn(const char *s, size_t len);

#endif
