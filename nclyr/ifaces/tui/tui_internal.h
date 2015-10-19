#ifndef NCLYR_TUI_TUI_INTERNAL_H
#define NCLYR_TUI_TUI_INTERNAL_H

#include "tui.h"

#include "iface.h"
#include "config.h"
#include "player.h"
#include "windows/window.h"
#include "windows/statusline.h"
#include "cmd_exec.h"

struct tui_iface {
    struct nclyr_iface iface;

    struct player_state_full state;

    struct nclyr_win **windows;
    int window_count;

    const struct nclyr_keypress *global_keys;

    int show_status :1;
    struct statusline *status;
    struct nclyr_win *manager_win;
    struct nclyr_win *help_win;

    int sel_window_index;
    struct nclyr_win *sel_window;

    int exit_flag;
    struct config_item *cfg;

    enum nclyr_mouse_event_type last_mevent;

    /* For command input bar. If 'grab_input' is set, then we're capturing the
     * keyboard to read in a command. If it's not set, then we're displaying
     * 'display' on the bottom bar. */
    unsigned int grab_input :1;
    char *inp_buf;
    char *display;
    size_t inp_buf_len;

    const struct tui_cmd *cmds;
};

void tui_change_window(struct tui_iface *, struct nclyr_win *);
void tui_lookup_song(struct tui_iface *, struct nclyr_win *);

void tui_config_init(struct config_item *);
struct config_item *tui_config_get_root(void);

enum tui_config_playlist {
    TUI_CONFIG_PLAYLIST_PRINTLINE,
    TUI_CONFIG_PLAYLIST_SONG_TRIPLE,
    TUI_CONFIG_PLAYLIST_SONG_FILENAME,
    TUI_CONFIG_PLAYLIST_TOTAL
};

enum tui_config_statusline {
    TUI_CONFIG_STATUSLINE_SONG,
    TUI_CONFIG_STATUSLINE_SONG_TRIPLE,
    TUI_CONFIG_STATUSLINE_SONG_FILENAME,
    TUI_CONFIG_STATUSLINE_TOTAL
};

enum tui_config_mpd_visualizer {
    TUI_CONFIG_MPD_VISUALIZER_FILENAME,
    TUI_CONFIG_MPD_VISUALIZER_COLOR,
    TUI_CONFIG_MPD_VISUALIZER_TOTAL
};

enum tui_config_mpd {
#if CONFIG_TUI_MPD_VISUALIZER
    TUI_CONFIG_MPD_VISUALIZER,
#endif
    TUI_CONFIG_MPD_TOTAL
};

struct tui_window_desc {
    const char *name;
    const char *player; /* If not null, then it requires the player matching this name */
    struct nclyr_win *(*new)(void);
};

extern struct tui_window_desc window_descs[];

#define WIN_DESC(wname, wplayer, winit) { .name = (wname), .player = (wplayer), .new = (winit) }
#define WIN_DESC_END() { .name = NULL }

void tui_window_init_dim(struct tui_iface *tui, struct nclyr_win *win, int rows, int cols, int y, int x);
void tui_window_init(struct tui_iface *tui, struct nclyr_win *win);
struct nclyr_win *tui_window_new(struct tui_iface *tui, struct tui_window_desc *win_desc);
struct nclyr_win *tui_window_new_dim(struct tui_iface *tui, struct tui_window_desc *win_desc, int rows, int cols, int y, int x);
void tui_window_add(struct tui_iface *tui, struct nclyr_win *win);
void tui_window_del(struct tui_iface *tui, int window_id);

#endif
