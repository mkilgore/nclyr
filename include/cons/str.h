#ifndef INCLUDE_CONS_STR_H
#define INCLUDE_CONS_STR_H

#include "nclyr_ncurses.h"
#include "color.h"

struct cons_str {
    int max_width;
    int alloced, length;
    cons_char *chstr;
};

void cons_str_setwidth(struct cons_str *, int width);

void cons_str_add_cons_str_at(struct cons_str *, const struct cons_str *str, int at);
void cons_str_add_cons_str(struct cons_str *dest, const struct cons_str *src);

void cons_str_add_str(struct cons_str *, const char *str, cons_char attrs);

void cons_str_add_ch(struct cons_str *, cons_char ch);

void cons_str_init(struct cons_str *);
void cons_str_clear(struct cons_str *);

#define make_cons_char(ch) ((chtype)(ch))

#endif
