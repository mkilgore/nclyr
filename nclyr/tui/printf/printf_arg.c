
#include "common.h"

#include <string.h>
#include <ncurses.h>

#include "tui_internal.h"
#include "tui_printf.h"
#include "compiler.h"
#include "args/arg_int.h"
#include "args/arg_string.h"
#include "printf_arg.h"
#include "debug.h"

struct printf_opt *printf_arg_get(int index, char *id_par, size_t arg_count, const struct tui_printf_arg *args)
{
    struct printf_opt *(*parse[])(int, char *, size_t, const struct tui_printf_arg *) = {
        [TUI_ARG_INT] = printf_arg_parse_int,
        [TUI_ARG_STRING] = printf_arg_parse_string,
    };

    return (parse[args[index].type]) (index, id_par, arg_count, args);
}

