#ifndef NCLYR_CONFIG_CONFIG_LEXER_H
#define NCLYR_CONFIG_CONFIG_LEXER_H

#include <stdio.h>

/*
enum config_token {
    TOK_STRING,
    TOK_EQUAL,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_COMMA,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_EOF,
    TOK_UNKNOWN
};

struct config_lexer {
    int line;
};

enum config_token yylex(struct config_lexer *); */

extern int yylex(void);

extern FILE *yyin;
extern int yylex_destroy(void);

#endif
