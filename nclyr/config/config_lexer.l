%top {
#include "common.h"
}

%{
#include <string.h>

#include "config_lexer.h"

#define YY_NO_INPUT
#define YY_DECL enum config_token yylex (struct config_lexer *state)
%}

%option noyywrap

%x qstring

%%

[ \t]+ /* Eat whitespace */

\n     state->line++;

("#"|"//")[^\n]*    /* One line comment */

"(" { return TOK_LPAREN; }
")" { return TOK_RPAREN; }
"," { return TOK_COMMA; }
"{" { return TOK_LBRACE; }
"}" { return TOK_RBRACE; }
"=" { return TOK_EQUAL; }

[a-zA-Z0-9_\-]+ {
    return TOK_STRING;
}

"\"" {
    BEGIN(qstring);
}

<qstring>[^\"]+ {
    return TOK_STRING;
}

<qstring>\" {
    BEGIN(INITIAL);
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

