#ifndef NCLYR_ARGS_H
#define NCLYR_ARGS_H

#include "config.h"

#define NCLYR_DEFAULT_ARGS \
    X(help, "help", 'h', 0, NULL, "Display help"), \
    X(help_all, "help-all", 'H', 0, NULL, "Display complete help information"), \
    X(version, "version", 'v', 0, NULL, "Display version information"), \
    X(list_players, "list-players", '\0', 0, NULL, "List available players"), \
    X(interface, "interface", 'i', 1, "Interface", "Interface to use ('console', 'tui', etc...)"), \
    X(list_interfaces, "list-interfaces", '\0', 0, NULL, "List available interfaces"), \
    X(config, "config", 'c', 1, "Filename", "Configuration file to use, default is ~/nclyrrc")

enum arg_index {
    ARG_EXTRA = ARG_PARSER_EXTRA,
    ARG_ERR = ARG_PARSER_ERR,
    ARG_DONE = ARG_PARSER_DONE,
#define X(enu, ...) ARG_##enu
    NCLYR_DEFAULT_ARGS,
#undef X
    ARG_LAST
};

extern const struct arg nclyr_args[];

#endif
