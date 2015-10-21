
%{
#include <string.h>

#include "cons/color.h"

#include "output_config.h"
#include "config_parser.h"
#include "config_parser.tab.h"

extern YYLTYPE yylloc;
#define YY_USER_INIT yylloc.first_line = 1;

#define YY_NO_INPUT

#define YY_USER_ACTION { \
        yylloc.first_line = yylineno; \
    }
%}

%option noyywrap
%option yylineno

%x inc

%%

[ \t\n]+ /* Eat whitespace */

("#"|"//")[^\n]*    /* One line comment */

"=" { return '='; }
"{" { return '{'; }
"}" { return '}'; }
"," { return ','; }
"(" { return '('; }
")" { return ')'; }

"include" {
    BEGIN(inc);
}

<inc>[ \t]* /* Whitespace */
<inc>[^ \t\n]+ {
    yyin = fopen(yytext, "r");
    if (!yyin) {
        fprintf(stderr, "Lexer: %d: include file '%s' not found\n", yylloc.first_line, yytext);
        return TOK_ERR;
    }

    yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
    BEGIN(INITIAL);
}

y {
    return 'y';
}

n {
    return 'n';
}

[0-9]+ {
    yylval.str = strdup(yytext);
    return TOK_INTEGER;
}

[a-zA-Z0-9_\-]+ {
    yylval.str = strdup(yytext);
    return TOK_IDENT;
}

\".*\" {
    yylval.str = strndup(yytext + 1, yyleng - 2);
    return TOK_STRING;
}

<<EOF>> {
    yypop_buffer_state();
    if (!YY_CURRENT_BUFFER)
        return TOK_EOF;
}

. {
    ;
}


%%

void fake_func(void)
{
    yyunput(0, 0);
}

