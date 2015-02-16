%top {
#include "common.h"
}

%{
#include <string.h>

#include "config_lexer.h"
#include "config_parser.h"

#define YY_NO_INPUT
%}

%option noyywrap
%option yylineno

%x qstring

%%

[ \t\n]+ /* Eat whitespace */
 state->line++;

("#"|"//")[^\n]*    /* One line comment */

"(" { return '('; }
")" { return ')'; }
"," { return ','; }
"{" { return '{'; }
"}" { return '}'; }
"=" { return '='; }

[tT][rR][uU][eE] {
    yylval.ival = 1;
    return TOK_BOOL;
}

[fF][aA][lL][sS][eE] {
    yylval.ival = 0;
    return TOK_BOOL;
}

[a-zA-Z0-9_\-]+ {
    yylval.str = strdup(yytext);
    return TOK_IDENT;
}

"\"" {
    BEGIN(qstring);
}

<qstring>[^\"]+ {
    yylval.str = strdup(yytext);
    return TOK_STRING;
}

<qstring>\" {
    BEGIN(INITIAL);
}


<<EOF>> {
    return TOK_EOF;
}

. {
    printf("Unexpected token %s\n", yytext);
    return TOK_ERR;
}

%%

void fake_func(void)
{
    yyunput(0, 0);
}

