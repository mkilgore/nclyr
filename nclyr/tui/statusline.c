
#include "common.h"

#include <ncurses.h>

#include "tui.h"
#include "tui_internal.h"
#include "player.h"
#include "statusline.h"

void statusline_update(struct statusline *status)
{
    if (status->player_is_running) {
        werase(status->win);
        mvwprintw(status->win, 0, 0, "Song: %s by %s on %s", tui.cur_song.title, tui.cur_song.artist, tui.cur_song.album);
        wrefresh(status->win);
    } else {
        struct player *player = player_current();
        werase(status->win);
        mvwprintw(status->win, 0, 0, "%s is not currently running", player->name);
        wrefresh(status->win);
    }
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

void statusline_player_notif(struct statusline *status, const struct player_notification *notif)
{
    switch (notif->type) {
    case PLAYER_IS_UP:
        status->player_is_running = 1;
        break;
    default:
        break;
    }

}

struct statusline statusline = {
    .win = NULL,
    .init = statusline_init,
    .clean = statusline_clean,
    .resize = statusline_resize,
    .update = statusline_update,
    .player_notif = statusline_player_notif
};

