#ifndef NCLYR_ARGS_H
#define NCLYR_ARGS_H

#include "arg_parser.h"

#define NCLYR_DEFAULT_ARGS \
    X(help, "help", 'h', 0, NULL, "Display help"), \
    X(version, "version", 'v', 0, NULL, "Display version information"), \
    X(list_players, "list-players", '\0', 0, NULL, "List available players")

int parse_args(int argc, char **argv, const char **player, const char **command);

#endif
