
#include "common.h"

#include <ncurses.h>

#include "cons_color.h"
#include "tui_color.h"
#include "debug.h"

static int color_map_curses[] = {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    -1,
};

#define CALC_PAIR(f, b, c) ((f) + (b) * (c))

void tui_color_init(void)
{
    const int colors[] = {
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
    int i, k;
    /* If it's 64 or less, then we assume it has standard default colors */
    int count = (COLOR_PAIRS <= 64)? 8: 9;

    DEBUG_PRINTF("Color pairs: %d\n", COLOR_PAIRS);
    for (i = 0; i < count; i++)
        for (k = 0; k < count; k++)
            init_pair(CALC_PAIR(colors[i], colors[k], count), color_map_curses[i], color_map_curses[k]);
}

void tui_color_set(WINDOW *win, struct cons_color_pair colors)
{
    int rf = CONS_COLOR_UNHIGHLIGHT(colors.f), rb = CONS_COLOR_UNHIGHLIGHT(colors.b);
    int count = (COLOR_PAIRS <= 64)? 8: 9;

    if (COLOR_PAIRS <= 64) {
        if (rf == CONS_COLOR_DEFAULT)
            rf = CONS_COLOR_WHITE;
        if (rb == CONS_COLOR_DEFAULT)
            rb = CONS_COLOR_BLACK;
    }

    wattron(win, COLOR_PAIR(CALC_PAIR(rf, rb, count)));
    if (CONS_COLOR_IS_HIGHLIGHT(colors.f))
        wattron(win, A_BOLD);
}

void tui_color_unset(WINDOW *win, struct cons_color_pair colors)
{
    int rf = CONS_COLOR_UNHIGHLIGHT(colors.f), rb = CONS_COLOR_UNHIGHLIGHT(colors.b);
    int count = (COLOR_PAIRS <= 64)? 8: 9;

    if (COLOR_PAIRS <= 64) {
        if (rf == CONS_COLOR_DEFAULT)
            rf = CONS_COLOR_WHITE;
        if (rb == CONS_COLOR_DEFAULT)
            rb = CONS_COLOR_BLACK;
    }


    wattroff(win, COLOR_PAIR(CALC_PAIR(rf, rb, count)));
    if (CONS_COLOR_IS_HIGHLIGHT(colors.f))
        wattroff(win, A_BOLD);
}

