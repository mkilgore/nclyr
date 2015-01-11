#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

/* Inspired via the Linux-kernel macro 'container_of' */
#define container_of(ptr, type, member) \
    ((type *) ((char*)(ptr) - offsetof(type, member)))

#define STATIC_ASSERT3(cond, msg) typedef char msg[(cond)?1:-1]
#define STATIC_ASSERT2(cond, line) STATIC_ASSERT3(cond, static_assertion_at_line_##line)
#define STATIC_ASSERT1(cond, line) STATIC_ASSERT2(cond, line)
#define STATIC_ASSERT(cond) STATIC_ASSERT1(cond, __LINE__)

#define QQ(x) #x
#define Q(x) QQ(x)

#define TP2(x, y) x ## y
#define TP(x, y) TP2(x, y)

#endif
