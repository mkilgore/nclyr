
/* Helper program - Parses the config file and generates the config header, as
 * well as Makefile include */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "config_parser.tab.h"
#include "config_c.h"
#include "config_m.h"

int main(int argc, char **argv)
{
    int ret = 0;
    struct config_output *out; 

    switch (argv[1][0]) {
    case 'c':
        out = &c_output;
        break;
    case 'm':
        out = &m_output;
        break;
    default:
        return 0;
    }

    out->start();

    yyparse(out);

    out->end();

    return ret;
}

