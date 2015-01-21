#ifndef NCLYR_TUI_STATUSLINE_H
#define NCLYR_TUI_STATUSLINE_H

#include <ncurses.h>

#include "player.h"

struct statusline {
    WINDOW *win;

    void (*init) (struct statusline *, int cols);
    void (*clean) (struct statusline *);
    void (*resize) (struct statusline *, int cols);
    void (*update) (struct statusline *);
    void (*player_notif) (struct statusline *, const struct player_notification *);

    int player_is_running :1;
};

extern struct statusline statusline;

#endif
