
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

    for (i = 0; i < sizeof(colors)/sizeof(*colors); i++)
        for (k = 0; k < sizeof(colors)/sizeof(*colors); k++)
            init_pair(colors[i] + colors[k] * 8 + 1, color_map_curses[i], color_map_curses[k]);
}

void tui_color_set(WINDOW *win, struct cons_color_pair colors)
{
    int rf = CONS_COLOR_UNHIGHLIGHT(colors.f), rb = CONS_COLOR_UNHIGHLIGHT(colors.b);

    wattron(win, COLOR_PAIR(rf + rb * 8 + 1));
    if (CONS_COLOR_IS_HIGHLIGHT(colors.f))
        wattron(win, A_BOLD);
}

void tui_color_unset(WINDOW *win, struct cons_color_pair colors)
{
    int rf = CONS_COLOR_UNHIGHLIGHT(colors.f), rb = CONS_COLOR_UNHIGHLIGHT(colors.b);

    wattroff(win, COLOR_PAIR(rf + rb * 8 + 1));
    if (CONS_COLOR_IS_HIGHLIGHT(colors.f))
        wattroff(win, A_BOLD);
}

