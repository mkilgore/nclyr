#ifndef INCLUDE_CONS_COLOR_H
#define INCLUDE_CONS_COLOR_H

#include "nclyr_ncurses.h"

enum cons_color {
    CONS_COLOR_BLACK   = COLOR_BLACK,
    CONS_COLOR_RED     = COLOR_RED,
    CONS_COLOR_GREEN   = COLOR_GREEN,
    CONS_COLOR_YELLOW  = COLOR_YELLOW,
    CONS_COLOR_BLUE    = COLOR_BLUE,
    CONS_COLOR_MAGENTA = COLOR_MAGENTA,
    CONS_COLOR_CYAN    = COLOR_CYAN,
    CONS_COLOR_WHITE   = COLOR_WHITE,
    CONS_COLOR_DEFAULT = -1,
};

struct cons_color_pair {
    enum cons_color f;
    enum cons_color b;
};

typedef chtype cons_char;
typedef chtype cons_attr;

enum cons_color cons_color_get(const char *name);
const char *cons_color_name(enum cons_color color);

void cons_color_set_default(struct cons_color_pair *);
const struct cons_color_pair *cons_color_get_default(void);

int cons_color_pair_to_num(struct cons_color_pair *pair);
void cons_color_num_to_pair(int pair1, struct cons_color_pair *pair2);

#endif
