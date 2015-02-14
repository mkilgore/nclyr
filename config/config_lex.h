#ifndef INC_CONFIG_CONFIG_LEX_H
#define INC_CONFIG_CONFIG_LEX_H

#include <stdio.h>

extern FILE *yyin;
extern char *yytext;
extern int yylex_destroy(void);

extern int yylex(void);

#endif
