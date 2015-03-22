
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
#include "tui_color.h"
#include "cmd_exec.h"
#include "cmd_handle.h"
#include "windows/window.h"

#include "tui.h"
#include "debug.h"

void tui_keys_player(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct player *player = player_current();

    DEBUG_PRINTF("Got player key: %c\n", ch);

    switch (ch) {
    case ' ':
        player_toggle_pause(player);
        break;

    case 'p':
        player_prev(player);
        break;

    case 'n':
        player_next(player);
        break;

    case '+':
        player_change_volume(player, 1);
        break;

    case '-':
        player_change_volume(player, -1);
        break;
    }
}

void tui_keys_global(struct nclyr_win *win, int ch, struct nclyr_mouse_event *mevent)
{
    struct tui_iface *tui = win->tui;

    DEBUG_PRINTF("Got global key: %c\n", ch);
    if (ch == 'w') {
        tui->sel_window_index = (tui->sel_window_index + 1) % tui->window_count;
        tui_change_window(tui, tui->windows[tui->sel_window_index]);
    } else if (ch == 'q') {
        tui->sel_window_index = tui->sel_window_index - 1;
        if (tui->sel_window_index == -1)
            tui->sel_window_index = tui->window_count - 1;

        tui_change_window(tui, tui->windows[tui->sel_window_index]);
    } else if (ch == 'Q') {
        tui->exit_flag = 1;
    } else if (ch == ':') {
        tui->grab_input = 1;
        if (tui->display)
            free(tui->display);
        tui->display = NULL;
        memset(tui->inp_buf, 0, tui->inp_buf_len);
    }
}

