#ifndef INCLUDE_CONS_COLORS_H
#define INCLUDE_CONS_COLORS_H

#define CONS_COLOR_HI_FG (1 << 5)

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

#define CONS_COLOR_HIGHLIGHT(col) ((col) | CONS_COLOR_HI_FG)
#define CONS_COLOR_UNHIGHLIGHT(col) ((col) & ~(CONS_COLOR_HI_FG))
#define CONS_COLOR_IS_HIGHLIGHT(col) ((col) & CONS_COLOR_HI_FG)

struct cons_color_pair {
    enum cons_color f;
    enum cons_color b;
};

enum cons_color cons_color_get(const char *name);
const char *cons_color_name(enum cons_color color);

#endif
