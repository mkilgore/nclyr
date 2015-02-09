
#include "common.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stringcasecmp.h"
#include "filename.h"
#include "config_lexer.h"
#include "config.h"
#include "debug.h"


static int parse_item(const char *file, struct config_lexer *state, struct config_item *item)
{
    enum config_token tok;

    switch (item->type) {
    case CONFIG_STRING:
        tok = yylex(state);
        if (tok != TOK_STRING) {
            printf("%s: Expected a string on line %d\n", file, state->line);
            return 1;
        }

        config_item_data_clear(item->type, &item->u);
        rd_string_dup(&item->u.str, yytext);
        break;

    case CONFIG_BOOL:
        tok = yylex(state);
        if (tok != TOK_STRING) {
            printf("%s: Expected a string on line %d\n", file, state->line);
            return 1;
        }

        config_item_data_clear(item->type, &item->u);
        item->u.bol = (stringcasecmp(yytext, "true") == 0)? 1: 0;
        break;

    case CONFIG_INTEGER:
        tok = yylex(state);
        if (tok != TOK_STRING) {
            printf("%s: Expected a string on line %d\n", file, state->line);
            return 1;
        }

        config_item_data_clear(item->type, &item->u);
        item->u.integer = strtol(yytext, NULL, 0);
        break;

    case CONFIG_COLOR_PAIR:
        break;

    case CONFIG_GROUP:
        break;
    }

    return 0;
}

int config_load_from_file(struct root_config *root, const char *unexp_file)
{
    struct config_item *item;
    struct config_lexer state;
    enum config_token tok;
    char *file = filename_get(unexp_file);
    FILE *fin;
    int ret = 0;

    fin = fopen(file, "r");
    if (fin == NULL)
        return 1;

    memset(&state, 0, sizeof(state));

    yyin = fin;

    while ((tok = yylex(&state)) != TOK_EOF) {
        switch (tok) {
        case TOK_STRING:
            DEBUG_PRINTF("yytext: %s\n", yytext);
            item = config_item_find(root, yytext);
            if (item)
                DEBUG_PRINTF("Found item: %s\n", item->name);
            else
                break;

            tok = yylex(&state);

            if (tok == TOK_EQUAL) {
                if (parse_item(file, &state, item) != 0)
                    return 1;
            } else {
                printf("%s: Expected '=' on line %d\n", file, state.line);
                return 1;
            }

            break;

        case TOK_LPAREN:
        case TOK_RPAREN:
        case TOK_COMMA:
        case TOK_LBRACE:
        case TOK_RBRACE:
        case TOK_EQUAL:
            break;

        default:
        case TOK_UNKNOWN:
            printf("%s: Unknown token '%s' on line %d\n", file, yytext, state.line);
            ret = 1;
            break;
        }
    }

    free(file);
    fclose(fin);
    return ret;
}

