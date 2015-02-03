
#include "common.h"

#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#include "a_sprintf.h"
#include "tui.h"
#include "song.h"
#include "player.h"
#include "cons_color.h"
#include "tui_color.h"
#include "tui/window.h"
#include "playlist_win.h"
#include "debug.h"

static void handle_ch(struct nclyr_win *win, int ch)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->tui;
    int rows;

    switch (ch) {
    case 'k':
        if (play->selected > 0) {
            play->selected--;
            if (play->selected < play->disp_offset)
                play->disp_offset--;
            win->updated = 1;
        }
        break;
    case 'j':
        if (play->selected < tui->state.playlist.song_count - 1) {
            play->selected++;
            rows = getmaxy(win->win);
            if (play->selected > play->disp_offset + rows - 1)
                play->disp_offset++;
            win->updated = 1;
        }
        break;
    case '\n':
        player_change_song(player_current(), play->selected);
        break;
    }
}

static void playlist_win_update(struct nclyr_win *win)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    struct tui_iface *tui = win->tui;
    int rows, cols, i;
    WINDOW *curwin = win->win;

    win->updated = 0;

    werase(curwin);

    getmaxyx(curwin, rows, cols);

    for (i = 0; i < rows; i++) {
        if (tui->state.playlist.song_count > i + play->disp_offset) {
            enum cons_color fc = CONS_COLOR_DEFAULT, bc = CONS_COLOR_DEFAULT;
            struct song_info *song = tui->state.playlist.songs[i + play->disp_offset];
            char *name;

            if (i + play->disp_offset == tui->state.song_pos)
                fc = CONS_COLOR_HIGHLIGHT(CONS_COLOR_YELLOW);

            if (i + play->disp_offset == play->selected) {
                bc = CONS_COLOR_CYAN;
                if (fc == CONS_COLOR_DEFAULT)
                    fc = CONS_COLOR_BLACK;
            }

            tui_color_fb_set(curwin, fc, bc);

            a_sprintf(&name, "%s by %s on %s",
                    song->tag.title, song->tag.artist, song->tag.album);

            mvwprintw(curwin, i, 0, "%02d. %-*s",
                    i + play->disp_offset, cols - 4, name);

            free(name);

            tui_color_fb_unset(curwin, fc, bc);
        } else {
            mvwprintw(curwin, i, 0, "%*s", cols, "");
        }
    }
}

static void playlist_win_new_player_notif(struct nclyr_win *win, enum player_notif_type notif, struct player_state_full *state)
{
    struct playlist_win *play = container_of(win, struct playlist_win, super_win);
    if (notif == PLAYER_SONG_POS || notif == PLAYER_PLAYLIST)
        win->updated = 1;

    if (notif == PLAYER_PLAYLIST)
        play->selected = 0;

    return ;
}

struct playlist_win playlist_window = {
    .super_win = {
        .win_name = "Playlist",
        .win = NULL,
        .timeout = 400,
        .lyr_types = (const enum lyr_data_type[]) { -1 },
        .keypresses = (const struct nclyr_keypress[]) {
            { 'j', handle_ch, "Select song above" },
            { 'k', handle_ch, "Select song below" },
            { '\n', handle_ch, "Play selected song" },
            { '\0', NULL, NULL }
        },
        .init = NULL,
        .clean = NULL,
        .switch_to = NULL,
        .update = playlist_win_update,
        .resize = NULL,
        .clear_song_data = NULL,
        .new_song_data = NULL,
        .new_player_notif = playlist_win_new_player_notif,
    },
    .selected = 0,
    .disp_offset = 0
};

