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

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(*(arr)))

/* Sets the bit referred to by 'flag'. */
#define F(flag) (1 << (flag))

#define bit_set(flag, bit) (flag) = ((flag) | (1 << (bit)))
#define bit_clear(flag, bit) (flag) = ((flag) & ~(1 << (bit)))
#define bit_test(flag, bit) (((flag) & (1 << (bit))) == (1 << (bit)))
#define bit_get(flag, bit) ((flag) & (1 << (bit)))

struct rd_string {
    char *str;
    int was_malloced :1;
};

void rd_string_free(struct rd_string *);
void rd_string_dup(struct rd_string *, const char *s);

#include "gen_config.h"

#endif
