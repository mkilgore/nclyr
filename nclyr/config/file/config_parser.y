%{
#define IN_CONFIG_PARSER
#include "common.h"

#include <string.h>
#include <stdlib.h>

#include "stringcasecmp.h"
#include "config_lexer.h"
#include "config.h"
#include "config_parser.h"
#include "debug.h"

void yyerror(struct config_parser_state *state, const char *str);

static void prefix_add(struct config_parser_state *state, char *prefix);
static void prefix_remove(struct config_parser_state *state);

#define YYERROR_VERBOSE
%}

%union {
    char *str;
    int ival;
    struct config_item *item;
}

%parse-param { struct config_parser_state *state }

%token <str> TOK_IDENT
%token <str> TOK_STRING
%token <ival> TOK_INTEGER
%token <ival> TOK_BOOL
%token TOK_EOF TOK_ERR

%type <str> identifier
%type <str> string
%type <item> config_item

%start config_file

%%

config_file:
    assignment
    | config_file block
    | config_file assignment
    | config_file TOK_ERR {
        (void)@1.first_line;
        YYABORT;
    }
    | config_file TOK_EOF {
        YYACCEPT;
    }
    ;

block:
     prefix_start config_file prefix_end {
        prefix_remove(state);
     }
     ;

prefix_start:
    identifier '{' {
        prefix_add(state, $1);
        free($1);
    }

prefix_end:
    '}' {
        prefix_remove(state);
    }

assignment:
    config_item '=' string {
        config_item_data_clear($1->type, &$1->u);
        rd_string_dup(&$1->u.str, $3);

        free($3);
    }
    | config_item '=' TOK_INTEGER {
        config_item_data_clear($1->type, &$1->u);
        $1->u.integer = $3;
    }
    | config_item '=' TOK_BOOL {
        config_item_data_clear($1->type, &$1->u);
        $1->u.bol = $3;
    }
    ;

config_item:
    identifier {
        char *name = NULL;
        if (state->prefix) {
            name = malloc(strlen(state->prefix) + strlen($1) + 2);
            name[0] = '\0';
            strcat(name, state->prefix);
            strcat(name, "-");
            strcat(name, $1);
        } else {
            name = $1;
        }

        $$ = config_item_find(state->root, name);

        if (state->prefix)
            free(name);
        free($1);

        if (!$$)
            YYABORT;
    }
    ;

identifier:
    TOK_IDENT {
        $$ = $1;
    }
    ;

string:
    TOK_STRING {
        $$ = $1;
    }
    | string TOK_STRING {
        $$ = realloc($$, strlen($$) + strlen($2) + 2);
        strcat($$, $2);
        free($2);
    }
    ;

%%

static void prefix_add(struct config_parser_state *state, char *prefix)
{
    size_t len;
    if (state->prefix)
        len = strlen(state->prefix);
    else
        len = 0;

    state->prefix = realloc(state->prefix, len + strlen(prefix) + 2);
    state->prefix[len] = '\0';
    strcat(state->prefix, prefix);
    strcat(state->prefix, "-");
}

static void prefix_remove(struct config_parser_state *state)
{
    char *c = state->prefix + strlen(state->prefix) - 2;
    for (; c > state->prefix; c--) {
        if (*c == '-') {
            *(c + 1) = '\0';
            return ;
        }
    }
    state->prefix[0] = '\0';
}

void yyerror(struct config_parser_state *state, const char *str)
{
    fprintf(stderr, "Parser error: %d: %s\n", yylloc.first_line, str);
}

