
#include "common.h"

#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "lyr_thread.h"
#include "tui_state.h"

void tui_state_change_song (struct tui_state *tui, struct song_info *song)
{
    struct nclyr_win **win;

    song_clear(&tui->cur_song);
    song_copy(&tui->cur_song, song);
    for (win = tui->windows; *win; win++)
        (*win)->clear_song_data(*win);
}

void tui_state_change_window (struct tui_state *tui, struct nclyr_win *win)
{
    tui->sel_window = win;
    tui->sel_window->switch_to(tui->sel_window);

    if (!tui->sel_window->already_lookedup) {
        tui->sel_window->already_lookedup = 1;
        lyr_thread_song_lookup(&tui->cur_song, tui->sel_window->lyr_types);
    }
}

