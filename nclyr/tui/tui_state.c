
#include "common.h"

#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "tui_state.h"

void tui_state_change_song (struct tui_state *tui, struct song_info *song)
{
    int i;
    song_clear(&tui->cur_song);
    song_copy(&tui->cur_song, song);
    for (i = 0; i < tui->window_count; i++)
        tui->windows[i]->clear_song_data(tui->windows[i]);
}

void tui_state_change_window (struct tui_state *tui, int win)
{
    tui->sel_window = win;
    tui->windows[tui->sel_window]->switch_to(tui->windows[tui->sel_window]);


    if (!tui->windows[tui->sel_window]->already_lookedup) {
        tui->windows[tui->sel_window]->already_lookedup = 1;
        song_thread_song_lookup(&tui->cur_song, tui->windows[tui->sel_window]->types);
    }
}

