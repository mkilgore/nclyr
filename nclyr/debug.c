
#include "common.h"

#include <stdio.h>
#include <stdarg.h>

#include "debug.h"

static FILE *debugfile;

void debug_init(void)
{
    debugfile = fopen(DEBUG_FILE, "w");
    setvbuf(debugfile, NULL, _IONBF, 0);
}

void debug_close(void)
{
    fclose(debugfile);
}

void debug_printf(const char *file, const char *func, const char *line, const char *str, ...)
{
    va_list args;

    va_start(args, str);
    fprintf(debugfile, "%s: %s: %s: ", file, func, line);
    vfprintf(debugfile, str, args);
    va_end(args);

    return ;
}


