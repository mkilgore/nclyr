
#include "common.h"

#include <unistd.h>
#include <sys/poll.h>
#include <signal.h>
#include <ncurses.h>

#include "song.h"
#include "lyr_thread.h"
#include "windows/window.h"
#include "tui_internal.h"

void tui_lookup_song(struct tui_iface *tui, struct nclyr_win *win)
{
    if (!win->already_lookedup) {
        win->already_lookedup = 1;
        lyr_thread_song_lookup(tui->state.song, win->lyr_types);
        if (win->lookup_started)
            (win->lookup_started) (win);
    }
}

void tui_change_window (struct tui_iface *tui, struct nclyr_win *win)
{
    tui->sel_window = win;
    touchwin(tui->sel_window->win);
    if (tui->sel_window->switch_to)
        tui->sel_window->switch_to(tui->sel_window);

    tui_lookup_song(tui, tui->sel_window);
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

