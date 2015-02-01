
#include "common.h"

#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "lyr_thread.h"
#include "tui.h"

void tui_change_window (struct tui_iface *tui, struct nclyr_win *win)
{
    tui->sel_window = win;
    tui->sel_window->switch_to(tui->sel_window);

    if (!tui->sel_window->already_lookedup) {
        tui->sel_window->already_lookedup = 1;
        lyr_thread_song_lookup(tui->state.song, tui->sel_window->lyr_types);
    }
}

