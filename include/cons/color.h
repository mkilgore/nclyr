#ifndef INCLUDE_CONS_COLOR_H
#define INCLUDE_CONS_COLOR_H

#include "nclyr_ncurses.h"
#include "debug.h"

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

union cons_color_pair_num {
    unsigned char pair_num;
    struct {
        unsigned char f_def :1;
        unsigned char b_def :1;
        unsigned char fg    :3;
        unsigned char bg    :3;
    } p;
};

#define cons_color_pair_get_f_def(f) ((f) & 0x80)
#define cons_color_pair_set_f_def(f, b) do { (f) = (((f) & 0x7F) | ((b) << 7)); } while (0)
#define cons_color_pair_get_b_def(b) ((b) & 0x40)
#define cons_color_pair_set_b_def(f, b) do { (f) = (((f) & 0xBF) | ((b) << 6)); } while (0)

#define cons_color_set_fg(c, fg) do { (c) = ((c & 0xF8) | ((fg) - COLOR_BLACK)); } while (0)
#define cons_color_get_fg(c) ((c) & 0x07)

#define cons_color_set_bg(c, bg) do { (c) = ((c & 0xC7) | (((bg) - COLOR_BLACK) << 3)); } while (0)
#define cons_color_get_bg(c) (((c) & 0x38) >> 3)

static inline int cons_color_pair_to_num(struct cons_color_pair *pair)
{
    int n = 0;
    if (pair->f == CONS_COLOR_DEFAULT)
        cons_color_pair_set_f_def(n, 1);
    else
        cons_color_set_fg(n, pair->f);

    if (pair->b == CONS_COLOR_DEFAULT)
        cons_color_pair_set_b_def(n, 1);
    else
        cons_color_set_bg(n, pair->b);

    return n + 1;
}

static inline void cons_color_num_to_pair(int pair1, struct cons_color_pair *pair2)
{
    int n = pair1 - 1;

    if (cons_color_pair_get_f_def(n))
        pair2->f = CONS_COLOR_DEFAULT;
    else
        pair2->f = cons_color_get_fg(n);

    if (cons_color_pair_get_b_def(n))
        pair2->b = CONS_COLOR_DEFAULT;
    else
        pair2->b = cons_color_get_bg(n);
}

#endif
