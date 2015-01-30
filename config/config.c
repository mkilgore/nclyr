
/* Helper program - Parses the config file and generates the config header, as
 * well as Makefile include */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "config_lex.h"
#include "config_c.h"
#include "config_m.h"

int main(int argc, char **argv)
{
    enum config_token tok;
    struct config_output *out;
    struct config_lexer state;

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

    memset(&state, 0, sizeof(state));

    while ((tok = yylex(&state)) != TOK_EOF) {
        if (tok == TOK_STRING) {
            char *token_str = strdup(yytext);
            tok = yylex(&state);
            if (tok != TOK_EQUAL) {
                free(token_str);
                continue;
            }

            tok = yylex(&state);
            if (tok != TOK_STRING) {
                free(token_str);
                continue;
            }

            if (strcmp(yytext, "y") == 0)
                out->write_var(token_str, 1);
            else
                out->write_var(token_str, 0);

            free(token_str);
        }
    }

    out->end();

    return 0;
}

