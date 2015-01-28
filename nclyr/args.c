
#include "common.h"

#include <string.h>

#include "player.h"
#include "args.h"
#include "debug.h"

#define ARG(id, op, arg, arg_text, help_text) \
  { .lng = id, \
    .shrt = op, \
    .help_txt = help_text, \
    .has_arg = arg, \
    .arg_txt = arg_text \
  }

#define CREATE_ARG(enu, id, op, arg, arg_text, help_text) \
  [ARG_##enu] = ARG(id, op, arg, arg_text, help_text)

const struct arg nclyr_args[] = {
#define X(...) CREATE_ARG(__VA_ARGS__)
    NCLYR_DEFAULT_ARGS,
#undef X
    { NULL }
};

