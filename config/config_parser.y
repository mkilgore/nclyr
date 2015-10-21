
%{
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "cons/color.h"

#include "output_config.h"
#include "config_lex.h"
#include "config_parser.h"

void yyerror(struct config_parser_state *state, const char *str);

static void prefix_add(struct config_parser_state *state, char *prefix);
static void prefix_remove(struct config_parser_state *state);

#define INVALID_COLOR -2

static int lookup_color(const char *name);

struct color_pair_map {
    const char *id;
    int color;
};

#define MK_MAP(cname, cons) \
    { .id = cname, .color = cons }

static struct color_pair_map cmap[] = {
    MK_MAP("black", CONS_COLOR_BLACK),
    MK_MAP("red", CONS_COLOR_RED),
    MK_MAP("green", CONS_COLOR_GREEN),
    MK_MAP("yellow", CONS_COLOR_YELLOW),
    MK_MAP("blue", CONS_COLOR_BLUE),
    MK_MAP("magenta", CONS_COLOR_MAGENTA),
    MK_MAP("cyan", CONS_COLOR_CYAN),
    MK_MAP("white", CONS_COLOR_WHITE),
    MK_MAP("default", CONS_COLOR_DEFAULT),
};

#undef MK_MAP

#define YYERROR_VERBOSE
%}

%union {
    char *str;
    struct cons_color_pair pair;
}

%parse-param { struct config_parser_state *state }

%token <str> TOK_STRING
%token <str> TOK_IDENT
%token <str> TOK_INTEGER
%token TOK_EOF TOK_ERR

%type <str> string
%type <str> identifier
%type <pair> color_pair

%%

config:
      assignment
      | block
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
     prefix_start config prefix_end
     ;

prefix_start:
    identifier '{' {
        prefix_add(state, $1);
        free($1);
    }
    ;

prefix_end:
    '}' {
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
          | identifier '=' color_pair {
              char buf[50];
              sprintf(buf, "{ .f = %d, .b = %d}", $3.f, $3.b);
              state->out->write_var(OUTPUT_NOQUOTE_STRING, state->prefix, $1, buf);
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
      | TOK_IDENT {
          $$ = $1;
      }
      | string TOK_STRING {
          $1 = realloc($1, strlen($1) + strlen($2) + 1);
          strcat($1, $2);
          free($2);
          $$ = $1;
      }
      ;

color_pair:
    '(' TOK_IDENT ',' TOK_IDENT ')' {
        $$.f = lookup_color($2);
        $$.b = lookup_color($4);

        if ($$.f == INVALID_COLOR) {
            yyerror(state, $2);
            YYERROR;
        }

        if ($$.b == INVALID_COLOR) {
            yyerror(state, $4);
            YYERROR;
        }
    }

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

static int stringcasecmp(const char *s1, const char *s2)
{
    for (; *s1 && *s2; s1++, s2++)
        if (toupper(*s1) != toupper(*s2))
            return 1;

    if (*s1 || *s2)
        return 1;

    return 0;
}

static int lookup_color(const char *name)
{
    int c;
    for (c = 0; c < sizeof(cmap)/sizeof(*cmap); c++)
        if (stringcasecmp(name, cmap[c].id) == 0)
            return cmap[c].color;

    return INVALID_COLOR;
}

void yyerror(struct config_parser_state *state, const char *str)
{
    fprintf(stderr, "Parser error: %d: %s\n", yylloc.first_line, str);
}

