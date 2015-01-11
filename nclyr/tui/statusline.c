
#include "common.h"

#include <ncurses.h>

#include "tui.h"
#include "tui_internal.h"
#include "statusline.h"

void statusline_update(struct statusline *status)
{
    werase(status->win);
    mvwprintw(status->win, 0, 0, "Song: %s by %s on %s", tui.cur_song.title, tui.cur_song.artist, tui.cur_song.album);
    wrefresh(status->win);
}

void statusline_init(struct statusline *status, int cols)
{
    status->win = newwin(2, cols, 0, 0);
}

void statusline_clean(struct statusline *status)
{
    delwin(status->win);
}

void statusline_resize(struct statusline *status, int cols)
{
    wresize(status->win, 2, cols);
}

struct statusline statusline = {
    .win = NULL,
    .init = statusline_init,
    .clean = statusline_clean,
    .resize = statusline_resize,
    .update = statusline_update
};

