
#include "common.h"

#include <ncurses.h>

#include "tui.h"
#include "player.h"
#include "tui/statusline.h"
#include "debug.h"

void statusline_update(struct statusline *status)
{
    struct tui_iface *tui = status->tui;

    if (tui->state.is_up) {
        int cols, bar_len, x;
        werase(status->win);

        mvwprintw(status->win, 0, 0, "Song: %s by %s on %s",
                tui->state.song->title, tui->state.song->artist, tui->state.song->album);

        cols = getmaxx(status->win);

        bar_len = (int)((float)cols * ((float)tui->state.seek_pos / (float)tui->state.song->duration));

        DEBUG_PRINTF("Bar_len: %d\n", bar_len);

        mvwprintw(status->win, 0, cols - 13, "[%02d:%02d/%02d:%02d]",
                tui->state.seek_pos / 60, tui->state.seek_pos % 60,
                tui->state.song->duration / 60, tui->state.song->duration % 60);

        mvwprintw(status->win, 1, 0, "%.*s", cols, "");

        for (x = 0; x < bar_len; x++)
            mvwaddch(status->win, 1, x, '=');

        mvwaddch(status->win, 1, bar_len, '>');

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

void statusline_player_notif(struct statusline *status, enum player_notif_type notif, struct player_state_full *state)
{

}

struct statusline statusline = {
    .win = NULL,
    .init = statusline_init,
    .clean = statusline_clean,
    .resize = statusline_resize,
    .update = statusline_update,
    .player_notif = statusline_player_notif
};

