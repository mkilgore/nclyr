#ifndef NCLYR_TUI_STATUSLINE_H
#define NCLYR_TUI_STATUSLINE_H

#include <ncurses.h>

struct statusline {
    WINDOW *win;

    void (*init) (struct statusline *, int cols);
    void (*clean) (struct statusline *);
    void (*resize) (struct statusline *, int cols);
    void (*update) (struct statusline *);
};

extern struct statusline statusline;

#endif
