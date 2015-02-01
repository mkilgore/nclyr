
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#include "tui.h"
#include "song.h"
#include "player.h"
#include "cons_color.h"
#include "tui_color.h"
#include "tui/window.h"
#include "playlist_win.h"
#include "debug.h"

static void playlist_win_update(struct nclyr_win *win)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->tui;
    int rows, cols, i;
    WINDOW *curwin = win->win;

    werase(curwin);

    getmaxyx(curwin, rows, cols);

    for (i = 0; i < rows; i++) {
        if (tui->state.playlist.song_count > i + play->disp_offset) {
            int eq_flag;
            struct song_info *song = tui->state.playlist.songs[i];
            eq_flag = song_equal(tui->state.song, song);

            if (eq_flag) {
                DEBUG_PRINTF("Found match!\n");
                tui_color_fb_set(curwin, CONS_COLOR_RED, CONS_COLOR_DEFAULT);
            }

            mvwprintw(curwin, i, 0, "%02d. %s by %s on %s",
                    i + play->disp_offset,
                    song->title, song->artist, song->album);

            if (eq_flag)
                tui_color_fb_unset(curwin, CONS_COLOR_RED, CONS_COLOR_DEFAULT);
        } else {
            mvwprintw(curwin, i, 0, "%*s", cols, "");
        }
    }

    wrefresh(curwin);
}

static void playlist_win_init(struct nclyr_win *win, int y, int x, int rows, int cols)
{
    win->win = newwin(rows, cols, y, x);
}

static void playlist_win_clean(struct nclyr_win *win)
{
    delwin(win->win);
}

static void playlist_win_resize(struct nclyr_win *win, int y, int x, int rows, int cols)
{
    delwin(win->win);
    win->win = newwin(rows, cols, y, x);
    touchwin(win->win);
}

static void playlist_win_clear_song_data(struct nclyr_win *win)
{
    return ;
}

static void playlist_win_switch_to(struct nclyr_win *win)
{
    return ;
}

static void playlist_win_new_player_notif(struct nclyr_win *win, enum player_notif_type notif, struct player_state_full *state)
{
    return ;
}

struct playlist_win playlist_window = {
    .super_win = {
        .win_name = "Playlist",
        .win = NULL,
        .timeout = 500,
        .lyr_types = (const enum lyr_data_type[]) { -1 },
        .keypresses = (const struct nclyr_keypress[]) {
            { '\0', NULL, NULL }
        },
        .init = playlist_win_init,
        .clean = playlist_win_clean,
        .switch_to = playlist_win_switch_to,
        .update = playlist_win_update,
        .resize = playlist_win_resize,
        .clear_song_data = playlist_win_clear_song_data,
        .new_song_data = NULL,
        .new_player_notif = playlist_win_new_player_notif,
    },
    .disp_offset = 0
};


