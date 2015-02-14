
%{
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "config_lex.h"

void yyerror(struct config_output *out, const char *str);
%}

%union {
    char *str;
}

%parse-param { struct config_output *out }

%token <str> TOK_STRING
%token <str> TOK_IDENT
%token <str> TOK_INTEGER
%token TOK_EOF TOK_ERR

%type <str> string
%type <str> identifier

%%

config: assignment
      | config TOK_EOF  {
          YYABORT;
      }
      | config TOK_ERR {
          fprintf(stderr, "Line %d\n", @1.first_line);
      }
      | config assignment
      ;

assignment: identifier '=' string {
              out->write_var($1, 2, $3);
              free($1);
              free($3);
          }
          | identifier '=' TOK_INTEGER {
              out->write_var($1, 2, $3);
              free($1);
              free($3);
          }
          | identifier '=' 'y' {
              out->write_var($1, 1, NULL);
              free($1);
          }
          | identifier '=' 'n' {
              out->write_var($1, 0, NULL);
              free($1);
          }
          ;

identifier: TOK_IDENT {
              $$ = $1; 
          }
          ;

string: TOK_STRING {
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

void yyerror(struct config_output *out, const char *str)
{
    fprintf(stderr, "Parser error: %s\n", str);
}

