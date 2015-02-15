
/* Helper program - Parses the config file and generates the config header, as
 * well as Makefile include */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "config_parser.h"
#include "config_parser.tab.h"
#include "config_c.h"
#include "config_m.h"

int main(int argc, char **argv)
{
    int ret = 0;
    struct config_parser_state state;

    memset(&state, 0, sizeof(state));

    switch (argv[1][0]) {
    case 'c':
        state.out = &c_output;
        break;
    case 'm':
        state.out = &m_output;
        break;
    default:
        return 0;
    }

    state.out->start();

    if (yyparse(&state))
        ret = 1;

    state.out->end();

    return ret;
}

