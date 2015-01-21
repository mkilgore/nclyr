#ifndef NCLYR_CURSES_CLOCK_WIN_H
#define NCLYR_CURSES_CLOCK_WIN_H

#include "tui/window.h"

struct clock_win {
    struct nclyr_win super_win;

    int color, y, x, ydir, xdir;
    char *last_buf;
};

extern struct clock_win clock_window;

#endif
