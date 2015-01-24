#ifndef INCLUDE_A_SPRINTF_H
#define INCLUDE_A_SPRINTF_H

#include <stdarg.h>

int a_sprintfv (char **buf, const char *format, va_list lst);
int a_sprintf (char **buf, const char *format, ...);

#endif
