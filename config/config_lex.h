#ifndef INC_CONFIG_CONFIG_LEX_H
#define INC_CONFIG_CONFIG_LEX_H

#include <stdio.h>

enum config_token {
    TOK_STRING,
    TOK_EQUAL,
    TOK_EOF,
    TOK_UNKNOWN
};

struct config_lexer {
    int line;
};

enum config_token yylex(struct config_lexer *);

extern FILE *yyin;
extern char *yytext;
extern int yylex_destroy(void);

#endif
