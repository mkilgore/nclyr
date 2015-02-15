
%{
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "config_lex.h"
#include "config_parser.h"

void yyerror(struct config_parser_state *state, const char *str);

static void prefix_add(struct config_parser_state *state, char *prefix);
static void prefix_remove(struct config_parser_state *state);

#define YYERROR_VERBOSE
%}

%union {
    char *str;
}

%parse-param { struct config_parser_state *state }

%token <str> TOK_STRING
%token <str> TOK_IDENT
%token <str> TOK_INTEGER
%token TOK_EOF TOK_ERR

%type <str> string
%type <str> identifier

%%

config:
      assignment
      | config TOK_EOF  {
          YYACCEPT;
      }
      | config TOK_ERR {
          /* We have to use @1 somewhere in the parser to make yyac create the
           * yylloc variable. We need the yylloc variable for yyerror, but yyac
           * doesn't see usages of the yyloc variable outside of the parser, so
           * we stick a 'fake' usage here so yyac creates tye yylloc variable.
           */
          (void)@1.first_line; 
          YYABORT;
      }
      | config block
      | config assignment
      ;

block:
     identifier '{' {
        prefix_add(state, $1);
        free($1);
     }
     | '}' {
        prefix_remove(state);
     }
     ;

assignment:
          identifier '=' string {
              state->out->write_var(OUTPUT_QUOTE_STRING, state->prefix, $1, $3);
              free($1);
              free($3);
          }
          | identifier '=' TOK_INTEGER {
              state->out->write_var(OUTPUT_NOQUOTE_STRING, state->prefix, $1, $3);
              free($1);
              free($3);
          }
          | identifier '=' 'y' {
              state->out->write_var(OUTPUT_YES, state->prefix, $1, NULL);
              free($1);
          }
          | identifier '=' 'n' {
              state->out->write_var(OUTPUT_NO, state->prefix, $1, NULL);
              free($1);
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
          $1 = realloc($1, strlen($1) + strlen($2) + 1);
          strcat($1, $2);
          free($2);
          $$ = $1;
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
    strcat(state->prefix, "_");
}

static void prefix_remove(struct config_parser_state *state)
{
    char *c = state->prefix + strlen(state->prefix) - 2;
    for (; c > state->prefix; c--) {
        if (*c == '_') {
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

