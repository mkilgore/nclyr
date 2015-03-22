
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
#include "windows/clock_win.h"
#include "windows/lyrics_win.h"
#include "windows/help_win.h"
#include "windows/artist_win.h"
#include "windows/playlist_win.h"
#include "windows/config_win.h"

#include "tui.h"
#include "debug.h"

static struct nclyr_win *nclyr_windows[] = {
    &playlist_window.super_win,
#if CONFIG_LIB_GLYR
    &lyrics_window.super_win,
    &artist_window.super_win,
#endif
    &config_window.super_win,
    &help_window.super_win,
    &clock_window.super_win,
    NULL
};

struct tui_iface tui_iface  = {
    .iface = {
        .name = "tui",
        .description = "Text (ncurses-based) User Interface",
        .main_loop = tui_main_loop,
    },
    .state = { .is_up = 0 },
    .windows = nclyr_windows,
    .window_count = sizeof(nclyr_windows)/sizeof(*nclyr_windows) - 1,
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
    .sel_window_index = 0,
    .sel_window = NULL,
    .exit_flag = 0,
    .grab_input = 0,
    .cmds = (const struct tui_cmd[]) {
        TUI_CMD("player", tui_cmd_handle_player),
        TUI_CMD("window", tui_cmd_handle_window),
        TUI_CMD("tui",    tui_cmd_handle_tui),
        TUI_CMD("quit",   tui_cmd_quit),
        TUI_CMD_END()
    },
};

