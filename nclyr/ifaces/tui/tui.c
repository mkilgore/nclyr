
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "player.h"
#include "lyr_thread.h"
#include "tui_internal.h"
#include "tui_main.h"
#include "tui_keys.h"
#include "tui_cmds.h"
#include "tui_color.h"
#include "cmd_exec.h"
#include "cmd_handle.h"
#include "windows/window.h"
#include "windows/statusline.h"
#include "windows/manager_win.h"
#include "windows/clock_win.h"
#include "windows/lyrics_win.h"
#include "windows/help_win.h"
#include "windows/artist_win.h"
#include "windows/playlist_win.h"
#include "windows/config_win.h"
#include "windows/browse_win.h"
#include "windows/visual_win.h"

#include "tui.h"
#include "debug.h"

void tui_window_init_dim(struct tui_iface *tui, struct nclyr_win *w, int rows, int cols, int y, int x)
{
    w->tui = tui;
    w->win = newwin(rows, cols, y, x);
    touchwin(w->win);
    w->updated = 1;

    if (w->init)
        w->init(w);
}

void tui_window_init(struct tui_iface *tui, struct nclyr_win *win)
{
    int rows = getmaxy(tui->status->win);
    tui_window_init_dim(tui, win, LINES - rows - 2, COLS, rows + 1, 0);
}

struct nclyr_win *tui_window_new(struct tui_iface *tui, struct tui_window_desc *win_desc)
{
    int rows = getmaxy(tui->status->win);

    return tui_window_new_dim(tui, win_desc, LINES - rows - 2, COLS, rows + 1, 0);
}

struct nclyr_win *tui_window_new_dim(struct tui_iface *tui, struct tui_window_desc *win_desc, int rows, int cols, int y, int x)
{
    struct nclyr_win *w = (win_desc->new) ();

    tui_window_init_dim(tui, w, rows, cols, y, x);

    return w;
}

void tui_window_add(struct tui_iface *tui, struct nclyr_win *win)
{
    tui->window_count++;
    tui->windows = realloc(tui->windows, sizeof(*tui->windows) * tui->window_count);
    tui->windows[tui->window_count - 1] = win;
}

void tui_window_del(struct tui_iface *tui, int window_id)
{
    if (window_id + 1 < tui->window_count)
        memmove(tui->windows + window_id, tui->windows + window_id + 1, tui->window_count - window_id - 1);
    tui->window_count--;
}

struct tui_window_desc window_descs[] = {
    WIN_DESC("playlist", NULL, playlist_win_new, 0
            | F(PN_HAS_PLAYLIST)
            ,
            0),
#if CONFIG_LIB_GLYR
    WIN_DESC("lyrics", NULL, lyrics_win_new, 0
            | F(PN_HAS_SONG)
            ,
            0),
    WIN_DESC("artist", NULL, artist_win_new, 0
            | F(PN_HAS_SONG)
            ,
            0),
#endif
    WIN_DESC("browse", NULL, browse_win_new, 0
            | F(PN_HAS_DIRECTORY)
            ,
            0
            | F(PC_HAS_GET_DIRECTORY)),
    WIN_DESC("config", NULL, config_win_new, 0, 0),
    WIN_DESC("clock", NULL, clock_win_new, 0, 0),
#if CONFIG_TUI_MPD_VISUALIZER
    WIN_DESC("visual", "mpd", visual_win_new, 0, 0),
#endif
    WIN_DESC_END()
};

struct tui_iface tui_iface  = {
    .iface = {
        .name = "tui",
        .description = "Text (ncurses-based) User Interface",
        .main_loop = tui_main_loop,
    },
    .state = PLAYER_STATE_FULL_INIT(tui_iface.state),
    .windows = NULL,
    .window_count = 0,
    .global_keys = (const struct nclyr_keypress[]) {
        N_KEYPRESS('q', tui_keys_global, "Switch to previous window."),
        N_KEYPRESS('w', tui_keys_global, "Switch to next window."),
        N_KEYPRESS('Q', tui_keys_global, "Exit TUI."),
        N_KEYPRESS(':', tui_keys_global, "Start command entry."),

        N_KEYPRESS(' ', tui_keys_player, "Toggle Pause"),
        N_KEYPRESS('p', tui_keys_player, "Previous song"),
        N_KEYPRESS('n', tui_keys_player, "Next song"),
        N_KEYPRESS('+', tui_keys_player, "+1 volume"),
        N_KEYPRESS('-', tui_keys_player, "-1 volume"),

        N_END()
    },
    .show_status = 1,
    .status = &statusline,
    .manager_win = &manager_win.super_win,
    .help_win = &help_win.super_win,
    .sel_window_index = 0,
    .sel_window = NULL,
    .exit_flag = 0,
    .grab_input = 0,
    .cmds = (const struct tui_cmd[]) {
        TUI_CMD("player", tui_cmd_handle_player),
        TUI_CMD("window", tui_cmd_handle_window),
        TUI_CMD("tui",    tui_cmd_handle_tui),
        TUI_CMD("quit",   tui_cmd_quit),
        TUI_CMD("help",   tui_cmd_help),
        TUI_CMD_END()
    },
};

