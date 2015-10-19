#ifndef CONFIG_CONFIG_PARSER_H
#define CONFIG_CONFIG_PARSER_H

#include "output_config.h"

struct config_parser_state {
    struct config_output *out;
    char *prefix;
};

#endif
