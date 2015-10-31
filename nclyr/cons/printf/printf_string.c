
#include "common.h"

#include <string.h>

#include "cons/color.h"
#include "cons/str.h"
#include "cons/printf.h"
#include "compiler.h"
#include "debug.h"

static void print_string_clear(struct printf_opt *opt)
{
    free(opt->s);
    free(opt);
}

static struct printf_opt *printf_opt_str_new(void)
{
    struct printf_opt *po = malloc(sizeof(*po));
    memset(po, 0, sizeof(*po));
    po->type = PRINTF_OPT_STRING;
    po->print = NULL; /* Performed in cons_printf directly */
    po->clear = print_string_clear;
    return po;
}

struct printf_opt *print_string_get(const char *s)
{
    struct printf_opt *po = printf_opt_str_new();
    po->s = strdup(s);
    return po;
}

struct printf_opt *print_string_getn(const char *s, size_t len)
{
    struct printf_opt *po = printf_opt_str_new();
    po->s = strndup(s, len);
    return po;
}

