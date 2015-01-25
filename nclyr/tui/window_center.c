
#include "common.h"

#include <string.h>
#include <ncurses.h>

#include "window_center.h"

void win_center_str(WINDOW *win, const char *str)
{
    int rows, cols;
    int loc_r, loc_c;

    getmaxyx(win, rows, cols);

    loc_r = rows / 2;
    loc_c = cols / 2 - strlen(str) / 2;

    mvwprintw(win, loc_r, loc_c, "%s", str);
}

