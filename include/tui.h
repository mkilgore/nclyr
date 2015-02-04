#ifndef INCLUDE_TUI_H
#define INCLUDE_TUI_H

#include "iface.h"

struct tui_iface;

#include "config.h"
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
    struct config_item *cfg;
};

extern struct tui_iface tui_iface;

void tui_change_window(struct tui_iface *, struct nclyr_win *);

void tui_config_init(struct config_item *);
struct config_item *tui_config_get_root(void);

enum tui_config {
    TUI_CONFIG_PLAYLIST,
    TUI_CONFIG_STATUSLINE,
};

enum tui_config_playlist {
    TUI_CONFIG_PLAYLIST_SEL_PLAYING,
    TUI_CONFIG_PLAYLIST_PLAYING,
    TUI_CONFIG_PLAYLIST_SEL,
    TUI_CONFIG_PLAYLIST_NORMAL,
};

enum tui_config_statusline {
    TUI_CONFIG_STATUSLINE_SONG,
};

#endif
