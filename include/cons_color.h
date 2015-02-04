#ifndef INCLUDE_CONS_COLORS_H
#define INCLUDE_CONS_COLORS_H

enum cons_color {
    CONS_COLOR_BLACK,
    CONS_COLOR_RED,
    CONS_COLOR_GREEN,
    CONS_COLOR_YELLOW,
    CONS_COLOR_BLUE,
    CONS_COLOR_MAGENTA,
    CONS_COLOR_CYAN,
    CONS_COLOR_WHITE,
    CONS_COLOR_DEFAULT,
};

struct cons_color_pair {
    enum cons_color f;
    enum cons_color b;
};

enum cons_color cons_color_get(const char *name);
const char *cons_color_name(enum cons_color color);

#endif
