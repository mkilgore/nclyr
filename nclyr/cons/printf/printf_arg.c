
#include "common.h"

#include <string.h>

#include "cons/printf.h"
#include "compiler.h"
#include "args/arg_int.h"
#include "args/arg_string.h"
#include "args/arg_bool.h"
#include "args/arg_time.h"
#include "printf_arg.h"
#include "debug.h"

struct printf_opt *printf_arg_get(int index, char *id_par, size_t arg_count, const struct cons_printf_arg *args)
{
    struct printf_opt *(*parse[])(int, char *, size_t, const struct cons_printf_arg *) = {
        [CONS_ARG_INT] = printf_arg_parse_int,
        [CONS_ARG_STRING] = printf_arg_parse_string,
        [CONS_ARG_BOOL] = printf_arg_parse_bool,
        [CONS_ARG_TIME] = printf_arg_parse_time,
    };

    return (parse[args[index].type]) (index, id_par, arg_count, args);
}

