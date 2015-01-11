#ifndef INCLUDE_DEBUG_H
#define INCLUDE_DEBUG_H

#include "common.h"

void debug_init(void);
void debug_close(void);
void debug_printf(const char *file, const char *func, const char *line, const char *str, ...);

#define DEBUG_FILE "/tmp/nclyr_debug.txt"

#ifdef NCLYR_DEBUG

#define DEBUG_INIT() debug_init()
#define DEBUG_CLOSE() debug_close()
#define DEBUG_PRINTF(...) debug_printf(__FILE__, __func__, Q(__LINE__), __VA_ARGS__)

#else

#define DEBUG_INIT()
#define DEBUG_CLOSE()
#define DEBUG_PRINTF(...)

#endif

#endif
