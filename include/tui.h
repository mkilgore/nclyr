#ifndef INCLUDE_TUI_H
#define INCLUDE_TUI_H

#include "iface.h"

struct tui_iface;

#include "player.h"
#include "tui/window.h"
#include "tui/statusline.h"

struct tui_iface {
    struct nclyr_iface iface;

    struct player_state_full state;

    struct nclyr_win **windows;
    int window_count;

    const struct nclyr_keypress *global_keys;

    int show_status :1;
    struct statusline *status;

    int sel_window_index;
    struct nclyr_win *sel_window;

    int exit_flag;
};

extern struct tui_iface tui_iface;

void tui_change_window(struct tui_iface *, struct nclyr_win *);

#endif
