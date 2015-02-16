#ifndef NCLYR_CONFIG_CONFIG_PARSER_H
#define NCLYR_CONFIG_CONFIG_PARSER_H

#include "config.h"

struct config_parser_state {
    struct root_config *root;
    char *prefix;
};

#ifndef IN_CONFIG_PARSER
# include "config_parser.tab.h"
#endif

#endif
