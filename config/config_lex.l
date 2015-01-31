
%{
#include <string.h>

#include "config_lex.h"

#define YY_NO_INPUT
#define YY_DECL enum config_token yylex (struct config_lexer *state)
%}

%option noyywrap

%%

[ \t]+ /* Eat whitespace */

\n     state->line++;

("#"|"//")[^\n]*    /* One line comment */

"=" { return TOK_EQUAL; }

[a-zA-Z0-9_\-]+ {
    return TOK_STRING;
}

\".*\" {
    return TOK_STRING;
}

<<EOF>> {
    return TOK_EOF;
}

. {
    return TOK_UNKNOWN;
}


%%

void fake_func(void)
{
    yyunput(0, 0);
}

