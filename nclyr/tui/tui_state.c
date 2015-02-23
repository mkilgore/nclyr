
#include "common.h"

#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "lyr_thread.h"
#include "windows/window.h"
#include "tui_internal.h"

void tui_change_window (struct tui_iface *tui, struct nclyr_win *win)
{
    tui->sel_window = win;
    touchwin(tui->sel_window->win);
    if (tui->sel_window->switch_to)
        tui->sel_window->switch_to(tui->sel_window);

    if (!tui->sel_window->already_lookedup) {
        tui->sel_window->already_lookedup = 1;
        if (tui->state.song)
            lyr_thread_song_lookup(tui->state.song, tui->sel_window->lyr_types);
    }
}

static chtype attr_t_to_chtype(attr_t attrs)
{
    chtype attributes = 0;
    if (attrs & WA_BOLD)
        attributes |= A_BOLD;
    if (attrs & WA_REVERSE)
        attributes |= A_REVERSE;
    if (attrs & WA_DIM)
        attributes |= A_DIM;
    if (attrs & WA_BLINK)
        attributes |= A_BLINK;
    if (attrs & WA_UNDERLINE)
        attributes |= A_UNDERLINE;
    return attributes;
}

chtype tui_get_chtype_from_window(WINDOW *win)
{
    chtype ret = 0;
    int col_pair;
    attr_t attrs;

    wattr_get(win, &attrs, &col_pair, NULL);

    ret |= COLOR_PAIR(col_pair);
    ret |= attr_t_to_chtype(attrs);

    return ret;
}

