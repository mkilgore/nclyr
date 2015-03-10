
#include "common.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "filename.h"
#include "file/config_parser.h"
#include "file/config_lexer.h"
#include "config.h"
#include "debug.h"

int config_load_from_file(struct root_config *root, const char *unexp_file)
{
    struct config_parser_state state;
    char *file = filename_get(unexp_file);
    FILE *fin;
    int ret = 0;

    fin = fopen(file, "r");
    if (fin == NULL)
        return 1;

    memset(&state, 0, sizeof(state));

    state.root = root;

    yyin = fin;

    if (yyparse(&state) != 0)
        ret = 1;

    free(file);
    fclose(fin);
    return ret;
}

