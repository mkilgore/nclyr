#ifndef NCLYR_TUI_STATUSLINE_H
#define NCLYR_TUI_STATUSLINE_H

#include <ncurses.h>

struct statusline;

#include "cons/printf.h"
#include "player.h"
#include "tui.h"

struct statusline {
    WINDOW *win;

    struct tui_iface *tui;

    int updated :1;

    void (*init) (struct statusline *, int cols);
    void (*clean) (struct statusline *);
    void (*resize) (struct statusline *, int cols);
    void (*update) (struct statusline *);
    void (*player_notif) (struct statusline *, enum player_notif_type, struct player_state_full *);

    cons_printf_compiled *song_name;
    cons_printf_compiled *song_triple, *song_filename;
};

extern struct statusline statusline;

#endif
